#include <pmm.h>
#include <list.h>
#include <string.h>

/*
buddy_system system中的内存布局
低地址                              高地址
+--------------------------------------+
| |  |    |        |                   |
+--------------------------------------+
低地址的内存块较小             高地址的内存块较大
*/

extern const struct pmm_manager default_pmm_manager;
free_area_t free_area;

#define free_list (free_area.free_list)
#define nr_free (free_area.nr_free)

// 传入一个数，返回最接近该数的2的指数（包括该数为2的整数这种情况）
size_t getLessNearOfPower2(size_t size) 
{
  size |= size >> 1;
  size |= size >> 2;
  size |= size >> 4;
  size |= size >> 8;
  size |= size >> 16;
  //修改之后返回比size小的最大的二次幂
  size_t res = size+1;
  if(res > size) res = res>>1;
  return res;
}

static void
buddy_system_init(void) {
    list_init(&free_list);
    nr_free = 0;
}

static void
buddy_system_init_memmap(struct Page *base, size_t n) {
        assert(n>0);

    struct Page* p = base;

    for(;p!=base+n;p++){
        //表示这些页已经被使用了，将来不能被用于分配
        assert(PageReserved(p));
        p->flags = p->property = 0;
        set_page_ref(p, 0);
    }

    nr_free+=n;
    base +=n;

    //下面是从后往前切割空间,先分出大的再分出小的
    while(n){
        //设置一个大小为2次幂的块
        size_t curr_n = getLessNearOfPower2(n);
        base -= curr_n;
        base->property = curr_n;
        SetPageProperty(base);

        //根据块大小进行插入，从小到大
        //大小相等则看地址
        list_entry_t * le;
        for(le = list_next(&free_list);le!=&free_list;le = list_next(le)){
            struct Page *p = le2page(le, page_link);
            if(p->property > base->property
                || (p->property == base->property && base<p))
            break;
        }
        list_add_before(le,&(base->page_link));
        n -= curr_n;
    }
}

static struct Page *
buddy_system_alloc_pages(size_t n) {
    assert(n > 0);
    // 向上取2的幂次方，如果当前数为2的幂次方则不变
    size_t lessOfPower2 = getLessNearOfPower2(n);
    if (lessOfPower2 < n)
        n = 2 * lessOfPower2;
    // 如果待分配的空闲页面数量小于所需的内存数量
    if (n > nr_free) {
        return NULL;
    }
    // 查找符合要求的连续页
    //寻找符合要求的
    struct Page *page = NULL;
    list_entry_t *le = &free_list;
    //从空闲链表头开始查找最小的地址
    while ((le = list_next(le)) != &free_list) {
        //由链表元素获得对应的Page指针p
        struct Page *p = le2page(le, page_link);
        //满足大小，则返回page
        if (p->property >= n) {
            page = p;
            break;
        }
    }

    //因为建立的时候是从小到大,所以找到的肯定满足要求的最小的
    //且n也是2次幂，所以相等时必退出
    if (page != NULL) {
        //内存块过大，则连续切割内存
        while(page->property>n){
            page->property /= 2;
            struct Page * p = page + page->property;
            p->property = page->property;
            //这里不能忘记设置初始块的大小
            SetPageProperty(p);
            list_add_after(&(page->page_link), &(p->page_link)); 
        }
        
        nr_free -= n;
        ClearPageProperty(page);
        assert(page->property == n);
        list_del(&(page->page_link)); 
       
    }
    return page;
}

static void
buddy_system_free_pages(struct Page *base, size_t n) {
    assert(n > 0);
    // 向上取2的幂次方，如果当前数为2的幂次方则不变
    size_t lessOfPower2 = getLessNearOfPower2(n);
    if (lessOfPower2 < n)
        n = 2 * lessOfPower2;
    struct Page *p = base;
    for (; p != base + n; p ++) {
        assert(!PageReserved(p) && !PageProperty(p));
        p->flags = 0;
        set_page_ref(p, 0);
    }


    base->property = n;
    SetPageProperty(base);
    nr_free+=n;
    list_entry_t *le;

    //先插入到链表,根据大小插入
    //大小相等的时候根据地址
    for(le = list_next(&free_list);le!=&free_list;le=list_next(le)){
        p = le2page(le,page_link);
        //退出条件是base块大小<p的块大小,但是地址不一定按照大小
        //所以后面base就插入到p前面
        if((base->property <= p->property)
            ||(base->property == p->property && p>base))
        break;
    }
    list_add_before(le,&(base->page_link));

   //向前合并单个
    if(base->property==p->property && p+p->property == base){
        p->property += base->property;
        ClearPageProperty(base);
        list_del(&(base->page_link));
        base = p;
        le = &(base->page_link);
    }

    // 之后循环向后合并
    // 此时的le指向插入块的下一个块
    while (le != &free_list) {
        p = le2page(le, page_link);
        // 如果可以合并(大小相等+地址相邻),则合并
        // 如果两个块的大小相同，则它们不一定内存相邻。只有当这两个块都属于同一个大内存块时才算。
        // 也就是说，在一条链上，可能存在多个大小相等但却无法合并的块
        //大小相等且地址相邻则合并
        if(base->property == p->property && base+base->property == p){
            base->property += p->property;
            ClearPageProperty(p);
            list_del(&(p->page_link));
            le = &(base->page_link);
        }
        // 如果遍历到的内存块一定无法合并，则退出
        else if(base->property < p->property)
        {
                //使大小一样的放一起
            list_entry_t * targetLe = list_next(&base->page_link);
            //p = le2page(targetLe,page_link);

            //从前向后找 p中块大小 >= base中块大小
            //或者大小相等时,p<=base 时退出
            while(p->property < base->property || (p->property == base->property && p>base))
                targetLe = list_next(targetLe);
            
            //如果二者不相邻,base加到target之前
            if(targetLe != list_next(&base->page_link)){
                list_del(&(base->page_link));
                list_add_before(targetLe,&(base->page_link));
            }
            break;
        }
        le = list_next(le);
    }
}

static void
basic_check(void) {
    struct Page *p0, *p1, *p2;
    p0 = p1 = p2 = NULL;
    assert((p0 = alloc_page()) != NULL);
    assert((p1 = alloc_page()) != NULL);
    assert((p2 = alloc_page()) != NULL);

    assert(p0 != p1 && p0 != p2 && p1 != p2);
    assert(page_ref(p0) == 0 && page_ref(p1) == 0 && page_ref(p2) == 0);

    assert(page2pa(p0) < npage * PGSIZE);
    assert(page2pa(p1) < npage * PGSIZE);
    assert(page2pa(p2) < npage * PGSIZE);

    list_entry_t free_list_store = free_list;
    list_init(&free_list);
    assert(list_empty(&free_list));

    unsigned int nr_free_store = nr_free;
    nr_free = 0;

    assert(alloc_page() == NULL);

    free_page(p0);
    free_page(p1);
    free_page(p2);
    assert(nr_free == 3);

    assert((p0 = alloc_page()) != NULL);
    assert((p1 = alloc_page()) != NULL);
    assert((p2 = alloc_page()) != NULL);

    assert(alloc_page() == NULL);

    free_page(p0);
    assert(!list_empty(&free_list));

    struct Page *p;
    assert((p = alloc_page()) == p0);
    assert(alloc_page() == NULL);

    assert(nr_free == 0);
    free_list = free_list_store;
    nr_free = nr_free_store;

    free_page(p);
    free_page(p1);
    free_page(p2);
}

static void
buddy_system_check(void) {
    int count = 0, total = 0;
    list_entry_t *le = &free_list;
    while ((le = list_next(le)) != &free_list) {
        struct Page *p = le2page(le, page_link);
        assert(PageProperty(p));
        count ++, total += p->property;
    }
    assert(total == nr_free_pages());

    basic_check();

    //分配26个page，会自动对齐至32
    struct Page *p0 = alloc_pages(26), *p1;
    assert(p0 != NULL);
    assert(!PageProperty(p0));

    list_entry_t free_list_store = free_list;
    list_init(&free_list);
    assert(list_empty(&free_list));
    assert(alloc_page() == NULL);

    unsigned int nr_free_store = nr_free;
    nr_free = 0;
    //.........................................................
    // 先释放
    free_pages(p0, 26);     // 32+  (-:已分配 +: 已释放)
    // 首先检查是否对齐2
    p0 = alloc_pages(6);    // 8- 8+ 16+
    p1 = alloc_pages(10);   // 8- 8+ 16-
    assert((p0 + 8)->property == 8);
    free_pages(p1, 10);     // 8- 8+ 16+
    assert((p0 + 8)->property == 8);
    assert(p1->property == 16);
    p1 = alloc_pages(16);   // 8- 8+ 16-
    // 之后检查合并
    free_pages(p0, 6);      // 16+ 16-
    assert(p0->property == 16);
    free_pages(p1, 16);     // 32+
    assert(p0->property == 32);

    p0 = alloc_pages(8);    // 8- 8+ 16+
    p1 = alloc_pages(9);    // 8- 8+ 16-
    free_pages(p1, 9);     // 8- 8+ 16+
    assert(p1->property == 16);
    assert((p0 + 8)->property == 8);
    free_pages(p0, 8);      // 32+
    assert(p0->property == 32);
    // 检测链表顺序是否按照块的大小排序的
    p0 = alloc_pages(5);
    p1 = alloc_pages(16);
    free_pages(p1, 16);
    assert(list_next(&(free_list)) == &((p1 - 8)->page_link));
    free_pages(p0, 5);
    assert(list_next(&(free_list)) == &(p0->page_link));

    p0 = alloc_pages(5);
    p1 = alloc_pages(16);
    free_pages(p0, 5);
    assert(list_next(&(free_list)) == &(p0->page_link));
    free_pages(p1, 16);
    assert(list_next(&(free_list)) == &(p0->page_link));

    // 还原
    p0 = alloc_pages(26);
    //.........................................................
    assert(nr_free == 0);
    nr_free = nr_free_store;

    free_list = free_list_store;
    free_pages(p0, 26);

    le = &free_list;
    while ((le = list_next(le)) != &free_list) {
        assert(le->next->prev == le && le->prev->next == le);
        struct Page *p = le2page(le, page_link);
        count --, total -= p->property;
    }
    assert(count == 0);
    assert(total == 0);
}

static size_t
buddy_system_nr_free_pages(void) {
    return nr_free;
}

const struct pmm_manager buddy_system_pmm_manager = {
    .name = "buddy_system_pmm_manager",
    .init = buddy_system_init,
    .init_memmap = buddy_system_init_memmap,
    .alloc_pages = buddy_system_alloc_pages,
    .free_pages = buddy_system_free_pages,
    .nr_free_pages = buddy_system_nr_free_pages,
    .check = buddy_system_check,
};