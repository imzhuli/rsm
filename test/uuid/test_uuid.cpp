#include <iostream>
#include <zec/Common.hpp>
#include <zec/String.hpp>
#include <zec/Byte.hpp>
#include <zec_ext/Utility/UUID.hpp>
#include <cmath>

using namespace std;
using namespace zec;

int main(int, char **)
{
	xUUID UUID;
	for (size_t i = 1; i < 10; ++i) {
		UUID.Generate();
		auto Data = UUID.GetData();
		cout << "Round: " << i << " UUID=" << StrToHex(Data, 16) << endl;
	}

  	return 0;
}
