#include <iostream>
#include <fstream>
#include <sstream>

#include <string>
#include <vector>
#include <array>

#include "zlib.h"

#include "image.hpp"

struct Data
{
	std::array<std::uint8_t, 8> header{};
};

struct ChunkData
{
	std::uint32_t length{};
	std::uint32_t type{};

	std::vector<std::uint8_t> data{};

	std::uint32_t CRC{};
};

struct IHDR
{
	std::uint32_t width{};
	std::uint32_t height{};
	std::array<std::uint8_t, 5> aspects{};
};
IHDR ihdr{};

void read_integer(std::ifstream& stream, std::uint32_t& integer)
{
	stream.read(reinterpret_cast<char*>(&integer), sizeof(integer));
	integer = 
	(
		(reinterpret_cast<char*>(&integer)[0] << (3 * 8)) |
		(reinterpret_cast<char*>(&integer)[1] << (2 * 8)) | 
		(reinterpret_cast<char*>(&integer)[2] << (1 * 8)) |
		(reinterpret_cast<char*>(&integer)[3] << (0 * 8))
	);
}

void read_chunk(std::ifstream& stream, ChunkData& chunk)
{
	read_integer(stream, chunk.length);

	stream.read(reinterpret_cast<char*>(&chunk.type), sizeof(chunk.type));

	chunk.data.resize(chunk.length);


	stream.read(reinterpret_cast<char*>(chunk.data.data()), chunk.data.size());

	read_integer(stream, chunk.CRC);
}

std::vector<std::uint8_t> concatenate_data(const std::vector<ChunkData>& chunks)
{
	std::vector<std::uint8_t> data{};

	for (const auto& i : chunks)
	{
		std::string type 
		{
			 static_cast<char>(*(reinterpret_cast<const char*>(&i.type) + 0)),
			 static_cast<char>(*(reinterpret_cast<const char*>(&i.type) + 1)),
			 static_cast<char>(*(reinterpret_cast<const char*>(&i.type) + 2)),
			 static_cast<char>(*(reinterpret_cast<const char*>(&i.type) + 3))
		};

		if (type == "IDAT")
			data.insert(data.end(), i.data.begin(), i.data.end());
	}

	return data;
}

std::uint32_t convert_uint8_to_uint32(const std::array<std::uint8_t, 4>& vars)
{
	return
	std::uint32_t{
		static_cast<std::uint32_t>(vars[0]) << (3 * 8) |
		static_cast<std::uint32_t>(vars[1]) << (2 * 8) |
		static_cast<std::uint32_t>(vars[2]) << (1 * 8) |
		static_cast<std::uint32_t>(vars[3]) << (0 * 8)
	};
}

int inflate(std::vector<std::uint8_t>& in, std::vector<std::uint8_t>& dest, std::uint32_t chunk_size = 16384)
{
	if (in.size() > 4'294'967'295 /*Overflows if image's size is over 4GB*/)
		throw std::runtime_error("ERROR::PNG_DEFLATE::Buffer overflow, size of file is superior to 4 GigaBytes");


	z_stream strm;
	
	/* allocate inflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;


	int ret;

	ret = inflateInit(&strm);
	if (ret != Z_OK)
		throw std::runtime_error("ERROR::PNG_DEFLATE::Cannot initialize inflate process on data: " + std::to_string(ret));

	/* decompress until deflate stream ends or end of file */
	std::vector<std::uint8_t> out(chunk_size);
	do 
	{
		strm.avail_in = static_cast<uInt>(in.size());
		
		strm.next_in = in.data();

		/* run inflate() on input until output buffer not full */
		do 
		{
			strm.avail_out = chunk_size;
			strm.next_out = out.data();
			ret = inflate(&strm, Z_NO_FLUSH);

			if (ret == Z_STREAM_ERROR)
				throw std::runtime_error("ERROR::PNG_DEFLATE::State not clobbered: " + std::to_string(ret));

			switch (ret) 
			{
			case Z_NEED_DICT:
				ret = Z_DATA_ERROR;
				[[fallthrough]];
			case Z_DATA_ERROR:
			case Z_MEM_ERROR:
				inflateEnd(&strm);
				throw std::runtime_error("ERROR::PNG_DEFLATE::Couldn't read data properly: " + std::to_string(ret));
			}
			
			std::uint16_t have = chunk_size - strm.avail_out;

			dest.insert(dest.end(), out.begin(), out.begin() + have);

		} while (strm.avail_out == 0);

		/* done when inflate() says it's done */
	} while (ret != Z_STREAM_END);

	/* clean up and return */
	inflateEnd(&strm);
	return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}


int main()
{
	try
	{
		fill::Image image{ FILE_PATH"image.png" };




		std::ofstream out;

		out.open(FILE_PATH"file.pam", std::ios::binary);

		std::string s{ "P7\nWIDTH " + std::to_string(image.getWidth()) + 
					   "\nHEIGHT " + std::to_string(image.getHeight()) + "\nDEPTH 8\nMAXVAL 255\nTUPLTYPE RGB_ALPHA\nENDHDR\n" };

		out.write(s.c_str(), s.size());


		out.write(reinterpret_cast<const char*>(image.data()), image.size());


		out.close();

	}
	catch (const std::runtime_error& e)
	{
		std::cerr << e.what() << '\n';
	}


	return 0;
}