#include <iostream>
#include <string>
#include <fstream>

#include <sstream>

#include <vector>
#include <array>

int main()
{
	std::ifstream file;

	file.open(FILE_PATH"image.png", std::ios::binary);

	std::string str{ };

	std::stringstream str_s{};

	std::array<std::uint8_t, 8> header;


	file.read(reinterpret_cast<char*>(header.data()), sizeof(header));

	if (header[0] == 0x89 &&
		header[1] == 0x50 &&
		header[2] == 0x4e &&
		header[3] == 0x47 &&
		header[4] == 0xd &&
		header[5] == 0xa &&
		header[6] == 0x1a &&
		header[7] == 0xa)
	{
		std::cout << "Everything is good!\n";
	}

	/*for(const auto& byte : bytes)
		std::cout << std::hex << std::showbase << +byte << '\n';*/



	/*while (std::getline(file, str))
	{
		std::cout << str << '\n';
	}*/

	

	file.close();

	return 0;
}