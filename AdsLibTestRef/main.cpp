
#include <Windows.h>
#include <TcAdsDef.h>
#include <TcAdsAPI.h>
#include <cstdint>

#include <iostream>

#pragma warning(push, 0)
#include <fructose/fructose.h>
#pragma warning(pop)
using namespace fructose;

void print(const AmsAddr &addr, std::ostream &out)
{
	out << "AmsAddr: "
		<< (int)addr.netId.b[0] << '.' << (int)addr.netId.b[1] << '.' << (int)addr.netId.b[2] << '.'
		<< (int)addr.netId.b[3] << '.' << (int)addr.netId.b[4] << '.' << (int)addr.netId.b[5] << ':'
		<< addr.port << '\n';
}

long testPortOpen(std::ostream &out)
{
	long port = AdsPortOpenEx();
	if (!port) {
		return 0;
	}

	AmsAddr addr;
	if (AdsGetLocalAddressEx(port, &addr)) {
		AdsPortCloseEx(port);
		return 0;
	}
	out << "Port: " << port << ' ';
	print(addr, out);
	return port;
}

struct TestAds : test_base < TestAds >
{
	std::ostream &out;

	TestAds(std::ostream& outstream)
		: out(outstream)
	{}

	void testAdsPortOpenEx(const std::string&)
	{
		static const size_t NUM_TEST_PORTS = 2;
		long port[NUM_TEST_PORTS];


		for (int i = 0; i < NUM_TEST_PORTS; ++i) {
			port[i] = testPortOpen(out);
			fructose_loop_assert(i, 0 != port[i]);
		}

		for (int i = 0; i < NUM_TEST_PORTS; ++i) {
			if (port[i]) {
				fructose_loop_assert(i, !AdsPortCloseEx(port[i]));
			}
		}

	}

	void testAdsReadReqEx2(const std::string&)
	{
		AmsAddr server{ { 192, 168, 0, 231, 1, 1 }, AMSPORT_R0_PLC_TC3 };
		const long port = AdsPortOpenEx();
		fructose_assert(0 != port);

		print(server, out);

		uint32_t group = 0x4020;
		uint32_t offset = 4;
		unsigned long bytesRead;
		uint32_t buffer;

		for (int i = 0; i < 10; ++i) {
			fructose_assert(0 == AdsSyncReadReqEx2(port, &server, group, offset, sizeof(buffer), &buffer, &bytesRead));
			fructose_assert(0 == buffer);
		}
		fructose_assert(0 == AdsPortCloseEx(port));
	}

	void testAdsReadDeviceInfoReqEx(const std::string&)
	{
		static const char NAME[] = "Plc30 App";
		AmsAddr server{ { 192, 168, 0, 231, 1, 1 }, AMSPORT_R0_PLC_TC3 };
		const long port = AdsPortOpenEx();
		fructose_assert(0 != port);

		for (int i = 0; i < 2; ++i) {
			AdsVersion version{ 0, 0, 0 };
			char devName[16 + 1]{};
			fructose_assert(0 == AdsSyncReadDeviceInfoReqEx(port, &server, devName, &version));
			fructose_assert(3 == version.version);
			fructose_assert(1 == version.revision);
			fructose_assert(1101 == version.build);
			fructose_assert(0 == strncmp(devName, NAME, sizeof(NAME)));

			out << "AdsSyncReadDeviceInfoReqEx() name: " << devName
				<< " Version: " << (int)version.version << '.' << (int)version.revision << '.' << (int)version.build << '\n';
		}
		fructose_assert(0 == AdsPortCloseEx(port));
	}

	void testAdsReadStateReqEx(const std::string&)
	{
		AmsAddr server{ { 192, 168, 0, 231, 1, 1 }, AMSPORT_R0_PLC_TC3 };
		const long port = AdsPortOpenEx();
		fructose_assert(0 != port);


		USHORT adsState;
		USHORT deviceState;
		fructose_assert(0 == AdsSyncReadStateReqEx(port, &server, &adsState, &deviceState));
		fructose_assert(0 == AdsPortCloseEx(port));

		out << "AdsSyncReadStateReqEx() adsState: " << (int)adsState
			<< " device: " << (int)deviceState << '\n';
	}
};

int main()
{
	std::ostream nowhere(0);
#if 0
	std::ostream& errorstream = nowhere;
#else
	std::ostream& errorstream = std::cout;
#endif
	TestAds adsTest(errorstream);
	adsTest.add_test("testAdsPortOpenEx", &TestAds::testAdsPortOpenEx);
	adsTest.add_test("testAdsReadReqEx2", &TestAds::testAdsReadReqEx2);
	adsTest.add_test("testAdsReadDeviceInfoReqEx", &TestAds::testAdsReadDeviceInfoReqEx);
	adsTest.add_test("testAdsReadStateReqEx", &TestAds::testAdsReadStateReqEx);
	adsTest.run();

	std::cout << "Hit ENTER to continue\n";
	std::cin.ignore();
}