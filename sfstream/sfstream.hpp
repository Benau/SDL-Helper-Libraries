#pragma once

#include <istream>
#include <string>
#include <ostream>
#include <ios>
#include <iostream>

#include <SDL.h>
#include <array>
#define OVERRIDE override

class sfbuf : public std::streambuf {
	static constexpr std::size_t BufferSize=256;

	protected:
	SDL_RWops *m_File=nullptr;
	char *m_IBuffer=nullptr;
	char *m_OBuffer=nullptr;

	int_type sync() override;
	int_type overflow(int_type c) override;
	int_type underflow() override;
	std::streampos seekoff(std::streamoff off, std::ios::seekdir way, std::ios::openmode which=std::ios::in | std::ios::out) override;
	std::streampos seekpos(std::streampos pos, std::ios::openmode which=std::ios::in | std::ios::out) override;

	public:
	sfbuf(SDL_RWops *File);
	sfbuf(std::string Filename, std::string FileOptions);
	~sfbuf() noexcept override;
	
	void open(SDL_RWops *File);
	void open(std::string Filename, std::string FileOptions);

};

class isfstream : private sfbuf, public std::istream {
	public:
	isfstream(SDL_RWops *File);
	isfstream(std::string Filename, std::string FileOptions);
	void open(SDL_RWops *File);
	void open(std::string Filename, std::string FileOptions);
};
class osfstream : private sfbuf, public std::ostream {
	public:
	osfstream() = default;
	explicit osfstream(SDL_RWops *File);
	explicit osfstream(std::string Filename, std::string FileOptions);
	void open(SDL_RWops *File);
	void open(std::string Filename, std::string FileOptions);
};

class iosfstream : private sfbuf, public std::iostream {
	public:
	iosfstream() = default;
	explicit iosfstream(SDL_RWops *File);
	explicit iosfstream(std::string Filename, std::string FileOptions);
	void open(SDL_RWops *File);
	void open(std::string Filename, std::string FileOptions);
};


class SDLStreamBuffer : public std::streambuf
{
static const size_t BUFFER_SIZE = 256;
protected:
    SDL_RWops* m_file;
    std::array<char, BUFFER_SIZE> m_in_buffer;
    std::array<char, BUFFER_SIZE> m_out_buffer;

    int_type sync() OVERRIDE;
    int_type overflow(int_type c) OVERRIDE;
    int_type underflow() OVERRIDE;
    std::streampos seekoff(std::streamoff off, std::ios::seekdir way,
                           std::ios::openmode which = std::ios::in | std::ios::out) OVERRIDE;
    std::streampos seekpos(std::streampos pos, std::ios::openmode which = std::ios::in | std::ios::out) OVERRIDE
    {
        return seekoff(pos, std::ios::beg, which);
    }
public:
    SDLStreamBuffer(SDL_RWops* file = NULL);
    ~SDLStreamBuffer();
    void open(SDL_RWops* file);
};

class SDLRWopsifstream : private SDLStreamBuffer, public std::istream
{
public:
    SDLRWopsifstream(SDL_RWops* file = NULL);
    void open(SDL_RWops* file);
};
