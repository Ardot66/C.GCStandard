extern size_t MetaTestsPassed, MetaTestsRun;

static inline void MetaTest(size_t testsPassed, size_t testsRun)
{
    MetaTestsRun += 1;
    MetaTestsPassed += testsPassed >= testsRun;
}