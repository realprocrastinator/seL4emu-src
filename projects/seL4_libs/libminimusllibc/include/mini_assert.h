#define mini_assert(x) ((void)((x) || (__mini_assert_fail(#x, __FILE__, __LINE__, __func__),0)))

_Noreturn void __mini_assert_fail (const char *, const char *, int, const char *);