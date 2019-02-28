#include <iostream>
#include <cstdio>
#include <fstream>

#define SECTOR_SIZE 512

char g_arr[SECTOR_SIZE];

void print_usage(char *filename)
{
	std::cout << "Usgage: " << filename << " <out> <file 1> <file 2> ... <file N>" << std::endl;
}

int change_sector_count(std::ofstream &out)
{
	size_t sector_size;

	out.seekp(0, std::ofstream::end);
	sector_size = out.tellp() / SECTOR_SIZE;

	out.seekp(5);
	out.write((const char*)&sector_size, 2);
}

int change_kernel64_sector_count(std::ofstream &out)
{
	size_t sector_size;
	std::streampos old_pos = out.tellp();

	out.seekp(0, std::ofstream::end);
	sector_size = out.tellp() / SECTOR_SIZE - 1;

	out.seekp(7);
	out.write((const char*)&sector_size, 2);

	out.seekp(old_pos);
}

int main(int argc, char *argv[])
{
	if (argc < 4)
	{
		print_usage(argv[0]);
		return -1;
	}

	std::ofstream out_file(argv[1], std::ofstream::binary | std::ofstream::out);


	unsigned short kernel64SectorCnt;
	for (int i = 2; i < argc; i++)
	{
		std::ifstream in_file(argv[i], std::ifstream::binary | std::ifstream::in);

		if (!in_file.is_open())
		{
			std::cerr << "Failed to open " << argv[i] << std::endl;

			out_file.close();
			std::remove(argv[1]);

			return -2;
		}

		out_file << in_file.rdbuf();

		size_t in_size = in_file.tellg();
		if (in_size % SECTOR_SIZE)
			out_file.write(g_arr, SECTOR_SIZE - (in_size % SECTOR_SIZE));

		if (i == 3)
			change_kernel64_sector_count(out_file);
	}

	change_sector_count(out_file);
	return 0;
}
