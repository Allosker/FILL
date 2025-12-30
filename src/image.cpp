#include "image.hpp"

#include <array>

// Utility Classes

struct Chunk
{
	std::uint32_t length{};
	std::uint32_t type{};

	std::vector<std::uint8_t> data{};

	std::uint32_t CRC{};
};

struct ColorType
{
	ColorType() = default;

	ColorType(std::uint8_t t)
		: type{ static_cast<Type>(t) }
	{
	}

	enum Type
		: std::uint8_t
	{
		Greyscale = 0,
		TrueColor = 2,
		Indexed_Color = 3,
		Greyscale_with_Alpha = 4,
		TrueColor_with_Alpha = 6
	};

	std::uint8_t asBytes()
	{
		switch (type)
		{
		case Greyscale:
			return 1;
			break;

		case TrueColor:
			return 3;
			break;

		case Indexed_Color:
			return 0;
			break;

		case Greyscale_with_Alpha:
			return 3;
			break;

		case TrueColor_with_Alpha:
			return 4;
			break;
		}
	}

	Type type;
};


// Utility functions 

void fill::read_uint32(std::ifstream& stream, std::uint32_t& integer) noexcept
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

std::uint32_t fill::uint8_as_uint32(std::uint8_t byte0, std::uint8_t byte1, std::uint8_t byte2, std::uint8_t byte3) noexcept
{
	return
	std::uint32_t
	{
			static_cast<std::uint32_t>(byte0) << (3 * 8) |
			static_cast<std::uint32_t>(byte1) << (2 * 8) |
			static_cast<std::uint32_t>(byte2) << (1 * 8) |
			static_cast<std::uint32_t>(byte3) << (0 * 8)
	};
}

std::string fill::uint32_as_string(std::uint32_t _string) noexcept
{
	return 
	{
		static_cast<char>(*(reinterpret_cast<const char*>(&_string) + 3)),
		static_cast<char>(*(reinterpret_cast<const char*>(&_string) + 2)),
		static_cast<char>(*(reinterpret_cast<const char*>(&_string) + 1)),
		static_cast<char>(*(reinterpret_cast<const char*>(&_string) + 0))
	};
}

std::vector<std::uint8_t> fill::inflate(const std::vector<std::uint8_t>& in, std::uint32_t chunk_size = 16384)
{
	if (in.size() > 4'294'967'295 /*Overflows if image's size is over 4GB*/)
		throw std::runtime_error("ERROR::PNG_DEFLATE::Buffer overflow, size of file is superior to 4 GigaBytes");
	if (in.size() <= 0)
		throw std::runtime_error("ERROR::PNG_DEFLATE::In buffer doesn't contain any data");


	std::vector<std::uint8_t> destination{};

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

		strm.next_in = const_cast<std::uint8_t*>(in.data());

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

			destination.insert(destination.end(), out.begin(), out.begin() + have);

		} while (strm.avail_out == 0);

		/* done when inflate() says it's done */
	} while (ret != Z_STREAM_END);

	/* clean up and return */
	inflateEnd(&strm);
	return destination;
}


// Image Class

fill::Image::Image(const std::filesystem::path& path_to_file)
{
	loadFromFile(path_to_file);
}


void fill::Image::loadFromFile(const std::filesystem::path& path_to_file)
{
	if (path_to_file.has_extension())
	{
		if (stricmp(".png", path_to_file.extension().string().c_str()))
			return loadFromPNG(path_to_file);

		// Add other files
	}

	throw std::runtime_error("ERROR::No compatible version of the program was found for the file: " + path_to_file.string());
}

void fill::Image::concatenate_images(Image&& image, bool concatenate_horizontaly = true)
{
	// ----- WARNING
	// NEED to add more options to convert images to formats they both support if they aren't the same
	// Good for now

	if (concatenate_horizontaly)
	{
		std::uint32_t new_width = width + image.getWidth();
		std::uint32_t new_height{};

		if (height > image.getHeight())
		{
			new_height = height;

			while (image.size() < size())
			{
				image.getImage().push_back(0);
			}
		}
		else
		{
			new_height = image.getHeight();

			while (size() < image.size())
			{
				image_data.push_back(0);
			}
		}

		// TODO: make function to make one image hold the same parameters as the other

		short bpp{ color_channel * bit_depth }; /*assume both images have the same values (for now)*/
		short width_bytes_1{ width * bpp };
		short width_bytes_2{ image.getWidth() * bpp };


		for (std::uint64_t index_byte{}; index_byte < image_data.size(); index_byte++)
		{
			if (index_byte % width_bytes_1 == 0) /*start of line*/
			{
				std::uint64_t line_index{ index_byte / width_bytes_1 };

				//image_data.insert(image_data.begin() + index_byte + width_bytes_1, )
			}
			
		}


		
	}
	else
	{
		height += image.getHeight();
		width = (width > image.getWidth() ? width : image.getWidth());



	}


}


void fill::Image::loadFromPNG(const std::filesystem::path& path_png)
{
	std::ifstream file{};
	file.open(path_png.string(), std::ios::binary);


	std::array<std::uint8_t, 8> header; 
	file.read(reinterpret_cast<char*>(header.data()), header.size()); /*fetch header*/

	if (header[0] == 0x89 &&
		header[1] == 0x50 &&
		header[2] == 0x4e &&
		header[3] == 0x47 &&
		header[4] == 0xd &&
		header[5] == 0xa &&
		header[6] == 0x1a &&
		header[7] == 0xa)
	{

		Chunk ihdr;
		read_PNGchunk(file, ihdr); /*fetch IHDR chunk*/

		if (uint32_as_string(ihdr.type) != "IHDR")
			throw std::runtime_error("ERROR::WRONG_TYPE::File doesn't correspond to the PNG standard::No corresponding IHDR chunk");

		// Fetch attributes
		width = uint8_as_uint32(ihdr.data[0], ihdr.data[1], ihdr.data[2], ihdr.data[3]);
		height = uint8_as_uint32(ihdr.data[4], ihdr.data[5], ihdr.data[6], ihdr.data[7]);
		bit_depth = ihdr.data[8];
		ColorType color_type = static_cast<ColorType>(ihdr.data[9]);
		compression_method = ihdr.data[10];
		filter_method = ihdr.data[11];
		interlace_method = ihdr.data[12];

		color_channel = color_type.asBytes();

		
		std::vector<uint8_t> raw_data{};
		// Fetch Different Chunks & Collect Data
		while (!file.eof())
		{
			Chunk chunk;
			read_PNGchunk(file, chunk);

			std::string type{ uint32_as_string(chunk.type) };

			if (type == "IDAT")
				raw_data.insert(raw_data.end(), chunk.data.begin(), chunk.data.end());
		}

		// Apply DEFLATE
		std::vector<std::uint8_t> decompressed_data{};
		decompressed_data = inflate(raw_data);

		// Process Data
		unfilter_PNG(decompressed_data);
	}
	else
		throw std::runtime_error("ERROR::WRONG_TYPE::PNG file couldn't be read properly::No proper header");

}

void fill::Image::read_PNGchunk(std::ifstream& stream, Chunk& chunk)
{
	read_uint32(stream, chunk.length);
	chunk.data.resize(chunk.length);
	
	read_uint32(stream, chunk.type);
	
	stream.read(reinterpret_cast<char*>(chunk.data.data()), chunk.data.size());

	read_uint32(stream, chunk.CRC);
}

void fill::Image::unfilter_PNG(std::vector<std::uint8_t>& filtered_data)
{
	short bpp{ color_channel * (bit_depth / 8) };

	std::uint32_t width_bytes{ width * bpp };

	std::uint8_t filter{};

	std::uint8_t a{}, b{}, c{};
	short p{}, pa{}, pb{}, pc{};

	std::uint8_t data{};

	
	for (size_t byte_index{}; byte_index < filtered_data.size(); byte_index++)
	{

		if (byte_index % width_bytes == 0)
		{
			filter = filtered_data[byte_index];
			filtered_data.erase(filtered_data.begin() + byte_index);
		}

		data = filtered_data[byte_index];


		switch (filter)
		{
		// Sub
		case 1:
			if (byte_index < bpp || (byte_index / bpp) % width == 0)
				a = 0;
			else
				a = image_data[byte_index - bpp];


			data += a;
			break;

		// Up
		case 2:
			if (byte_index < width_bytes)
				b = 0;
			else
				b = image_data[byte_index - width_bytes];


			data += b;
			break;

		// Average
		case 3:
			if (byte_index < bpp || (byte_index / bpp) % width == 0)
				a = 0;
			else
				a = image_data[byte_index - bpp];

			if (byte_index < width_bytes)
				b = 0;
			else
				b = image_data[byte_index - width_bytes];


			data += std::floor((a + b) / 2);
			break;

		// Paeth
		case 4:
			if (byte_index < bpp || (byte_index / bpp) % width == 0)
				a = 0;
			else
				a = image_data[byte_index - bpp];

			if (byte_index < width_bytes)
				b = 0;
			else
				b = image_data[byte_index - width_bytes];

			if ((byte_index < width_bytes) || (byte_index < bpp || (byte_index / bpp) % width == 0))
				c = 0;
			else
				c = image_data[(byte_index - width_bytes) - bpp];


			p = static_cast<short>(a) + static_cast<short>(b) - static_cast<short>(c);
			pa = std::abs(p - static_cast<short>(a));
			pb = std::abs(p - static_cast<short>(b));
			pc = std::abs(p - static_cast<short>(c));

			if (pa <= pb && pa <= pc)
				data += a;
			else if (pb <= pc)
				data += b;
			else
				data += c;

			break;

		default:
			break;
		}

		image_data.push_back(data);
	}
}
