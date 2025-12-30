#pragma once // image.hpp
// MIT
// Allosker - 2025
// ===================================================
// This file contains a class containing an interface for my very own PNG loader.
// Each image is treated as its own object for further modifications thereof.
// This class is subject to modifications and change in its design:
//	- Uses the library ZLIB for the DEFLATE algorithm.
//	- Is only capable of reading the most primitive forms of PNG images (basic formats) -- non interlaced images.
//	- Size of image cannot exceed 4GB.
//	- If enabled, can concatenate two images to form a new one (e.g. creation of an atlas)
//
// See: https://www.w3.org/TR/2003/REC-PNG-20031110
// ===================================================


// Reading files
#include <fstream>
#include <filesystem>
// Sorting data
#include <string>
#include <vector>
#include <cstdint>

// DEFLATE algorithm
#include "zlib.h"


namespace fill
{
	inline void read_uint32(std::ifstream& stream, std::uint32_t& integer) noexcept;

	inline std::uint32_t uint8_as_uint32(std::uint8_t byte0, std::uint8_t byte1, std::uint8_t byte2, std::uint8_t byte3) noexcept;
	inline std::string uint32_as_string(std::uint32_t __string) noexcept;

	inline int inflate(std::vector<std::uint8_t>& in, std::vector<std::uint8_t>& dest, std::uint32_t chunk_size = 16384);


	class Image
	{
	public:

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
				: type{static_cast<Type>(t)}
			{ }

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

		
	// == Constructors

		Image(const std::filesystem::path& path_to_file);

		Image() noexcept = default;


	// == Actors 

		void loadFromFile(const std::filesystem::path& path_to_file);


	// == Getters

		std::uint32_t getWidth() const noexcept { return width; }
		std::uint32_t getHeight() const noexcept { return height; }
		
		std::uint8_t getBitDepth() const noexcept { return bit_depth; }
		ColorType getColorType() const noexcept { return color_type; }

		// size in bytes
		const size_t size() const noexcept { return image_data.size(); }

		const std::uint8_t* data() const noexcept { return image_data.data(); }
		std::uint8_t* data() noexcept { return image_data.data(); }


	private: 
		/*Actor Functions*/

		void loadFromPNG(const std::filesystem::path& path_png);

		void read_PNGchunk(std::ifstream& stream, Chunk& chunk);

		void unfilter_PNG(std::vector<std::uint8_t>& filtered_data);



	private: /*Members*/


		std::vector<std::uint8_t> image_data{};

		std::uint32_t width{}, height{};
		std::uint8_t bit_depth{}, compression_method{}, filter_method{}, interlace_method{};
		
		ColorType color_type{};
	};


} // fill