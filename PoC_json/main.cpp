#include <gmock/gmock.h>

using namespace testing;

int main(void)
{
#ifdef _DEBUG
	InitGoogleMock();
	return RUN_ALL_TESTS();
#else
	return 0;
#endif
}