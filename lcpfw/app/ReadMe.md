app模块架构参照了chromium的ui/views/examples，将其example中放在test中实现的部分模块进行拷贝修改收归己用。
具体参照为：
    # task_environment用于维护线程或线程池的任务投递和执行对象
    # copy from base/test/task_environment.*
    app_task_environment.*
    
    # discardable_memory_allocator用于xxxx
    # base/test/test_discardable_memory_allocator.*
    discardable_memory_allocator.*