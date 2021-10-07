#include "sfstream.hpp"

void sfbuf::open(SDL_RWops *File) {
	m_File = File;
	m_IBuffer = new char[BufferSize];
	m_OBuffer = new char[BufferSize];

	setg(m_IBuffer, m_IBuffer + BufferSize, m_IBuffer + BufferSize);
	setp(m_OBuffer, m_OBuffer + BufferSize);
}

void sfbuf::open(std::string Filename, std::string FileOptions) {
	m_File = SDL_RWFromFile(Filename.c_str(), FileOptions.c_str());
	if(m_File == nullptr) {
		std::cout << "Failed to open file " << Filename << "\nSDL_Error: " << SDL_GetError() << '\n';
		return;
	}

	m_IBuffer = new char[BufferSize];
	m_OBuffer = new char[BufferSize];

	setg(m_IBuffer, m_IBuffer + BufferSize, m_IBuffer + BufferSize);
	setp(m_OBuffer, m_OBuffer + BufferSize);
}

sfbuf::sfbuf(SDL_RWops *File) {
	open(File);
}

sfbuf::sfbuf(std::string Filename, std::string FileOptions) {
	open(Filename, FileOptions);
}

sfbuf::~sfbuf() noexcept {
		sync();
		if(SDL_RWclose(m_File) == -1) {
			//i dunno, something happened
		}
		if(m_IBuffer) {
			delete[] m_IBuffer;
		}
		if(m_OBuffer) {
			delete[] m_OBuffer;
		}
	}

sfbuf::int_type sfbuf::sync() {
	if(pptr() > pbase()) {
		size_t AmountWritten = SDL_RWwrite(m_File, m_OBuffer, sizeof(char), pptr() - pbase());
		if(AmountWritten < size_t(pptr() - pbase())) {
			std::cout << "sync failed, amount written: " << AmountWritten << ", SDL_Error: " << SDL_GetError() << '\n';
			return -1;
		}
		setp(m_OBuffer, m_OBuffer + BufferSize);
	}
	return 0;
}

sfbuf::int_type sfbuf::overflow(sfbuf::int_type c) {
	sync();
	if(c != EOF) {
		*pptr() = c;
		pbump(1);
	}
	return c;
}

sfbuf::int_type sfbuf::underflow() {
	if(gptr() < egptr()) {
		return *gptr();
	}

	size_t ReadAmount = SDL_RWread(m_File, m_IBuffer, sizeof(char), BufferSize);

	if(ReadAmount == 0) {
		return EOF;
	}

	setg(m_IBuffer, m_IBuffer, m_IBuffer + ReadAmount);
	return static_cast<unsigned char>(*gptr());
}

std::streampos sfbuf::seekoff(std::streamoff off, std::ios::seekdir way, std::ios::openmode which/*=std::ios::in | std::ios::out*/) {
	int whence;
	switch(way) {
		case std::ios::beg:
			whence = RW_SEEK_SET;
			break;

		case std::ios::cur:
			whence = RW_SEEK_CUR;
			break;
		
		case std::ios::end:
			whence = RW_SEEK_END;
			break;
		
		default: break;
	}
	Sint64 pos = SDL_RWseek(m_File, off, whence);
	if(pos == -1) {
		return -1;
	}
	setg(m_IBuffer, m_IBuffer + BufferSize, m_IBuffer + BufferSize);
	setp(m_OBuffer, m_OBuffer + BufferSize);

	return pos;
}

std::streampos sfbuf::seekpos(std::streampos pos, std::ios::openmode which/*=std::ios::in | std::ios::out*/) {
	return seekoff(pos, std::ios::beg, which);
}

isfstream::isfstream(SDL_RWops *File) : sfbuf(File), std::istream(this) {}

isfstream::isfstream(std::string Filename, std::string FileOptions) : sfbuf(Filename, FileOptions), std::istream(this) {}

void isfstream::open(SDL_RWops *File) {
	sfbuf::open(File);
}
void isfstream::open(std::string Filename, std::string FileOptions) {
	sfbuf::open(Filename, FileOptions);
}


osfstream::osfstream(SDL_RWops *File) : sfbuf(File), std::ostream(this) {}
osfstream::osfstream(std::string Filename, std::string FileOptions) : sfbuf(Filename, FileOptions), std::ostream(this) {}

void osfstream::open(SDL_RWops *File) {
	sfbuf::open(File);
}
void osfstream::open(std::string Filename, std::string FileOptions) {
	sfbuf::open(Filename, FileOptions);
}


iosfstream::iosfstream(SDL_RWops *File) : sfbuf(File), std::iostream(this) {}
iosfstream::iosfstream(std::string Filename, std::string FileOptions) : sfbuf(Filename, FileOptions), std::iostream(this) {}

void iosfstream::open(SDL_RWops *File) {
	sfbuf::open(File);
}
void iosfstream::open(std::string Filename, std::string FileOptions) {
	sfbuf::open(Filename, FileOptions);
}

    inline std::istream& safeGetline(std::istream& is, std::string& t)
    {
        t.clear();

        // The characters in the stream are read one-by-one using a std::streambuf.
        // That is faster than reading them one-by-one using the std::istream.
        // Code that uses streambuf this way must be guarded by a sentry object.
        // The sentry object performs various tasks,
        // such as thread synchronization and updating the stream state.
        std::istream::sentry se(is, true);
        std::streambuf* sb = is.rdbuf();

        for(;;)
        {
            int c = sb->sbumpc();
            switch (c)
            {
            case '\n':
                return is;
            case '\r':
                if(sb->sgetc() == '\n')
                    sb->sbumpc();
                return is;
            case std::streambuf::traits_type::eof():
                // Also handle the case when the last line has no line ending
                if (t.empty())
                    is.setstate(std::ios::eofbit);
                return is;
            default:
                t += (char)c;
            }
        }
    }



// ============================================================================
SDLStreamBuffer::SDLStreamBuffer(SDL_RWops* file)
{
    m_file = NULL;
    if (file)
        open(file);
}   // SDLStreamBuffer

// ----------------------------------------------------------------------------
SDLStreamBuffer::~SDLStreamBuffer()
{
    sync();
    if (m_file)
        SDL_RWclose(m_file);
}   // ~SDLStreamBuffer

// ----------------------------------------------------------------------------
void SDLStreamBuffer::open(SDL_RWops* file)
{
    if (!file)
        return;
    if (m_file)
    {
        sync();
        SDL_RWclose(m_file);
    }

    m_file = file;
    m_in_buffer = {};
    m_out_buffer = {};

    setg(m_in_buffer.data(), m_in_buffer.data() + BUFFER_SIZE,
        m_in_buffer.data() + BUFFER_SIZE);
    setp(m_out_buffer.data(), m_out_buffer.data() + BUFFER_SIZE);
}   // open

// ----------------------------------------------------------------------------
std::streambuf::int_type SDLStreamBuffer::sync()
{
    if (pptr() > pbase())
    {
        size_t written = SDL_RWwrite(m_file, m_out_buffer.data(), sizeof(char),
            pptr() - pbase());
        if (written < size_t(pptr() - pbase()))
            return -1;
        setp(m_out_buffer.data(), m_out_buffer.data() + BUFFER_SIZE);
    }
    return 0;
}   // sync

// ----------------------------------------------------------------------------
std::streambuf::int_type SDLStreamBuffer::overflow(int_type c)
{
    sync();
    if (c != EOF)
    {
        *pptr() = c;
        pbump(1);
    }
    return c;
}   // overflow

// ----------------------------------------------------------------------------
std::streambuf::int_type SDLStreamBuffer::underflow()
{
    if (gptr() < egptr())
        return *gptr();

    size_t readed = SDL_RWread(m_file, m_in_buffer.data(), sizeof(char),
        BUFFER_SIZE);
    if (readed == 0)
        return EOF;

    setg(m_in_buffer.data(), m_in_buffer.data(), m_in_buffer.data() + readed);
    return static_cast<unsigned char>(*gptr());
}   // underflow

// ----------------------------------------------------------------------------
std::streampos SDLStreamBuffer::seekoff(std::streamoff off,
                                        std::ios::seekdir way,
                                        std::ios::openmode which)
{
    int whence = 0;
    switch (way)
    {
    case std::ios::beg:
        whence = RW_SEEK_SET;
        break;
    case std::ios::cur:
        whence = RW_SEEK_CUR;
        break;
    case std::ios::end:
        whence = RW_SEEK_END;
        break;
    default:
        break;
    }
    int64_t pos = SDL_RWseek(m_file, off, whence);
    if (pos == -1)
        return -1;

    setg(m_in_buffer.data(), m_in_buffer.data() + BUFFER_SIZE,
        m_in_buffer.data() + BUFFER_SIZE);
    setp(m_out_buffer.data(), m_out_buffer.data() + BUFFER_SIZE);
    return pos;
}   // seekoff

// ============================================================================
SDLRWopsifstream::SDLRWopsifstream(SDL_RWops* file)
                : SDLStreamBuffer(file), std::istream(this)
{
}   // SDLRWopsifstream

// ----------------------------------------------------------------------------
void SDLRWopsifstream::open(SDL_RWops* file)
{
    SDLStreamBuffer::open(file);
}   // open

int main()
{
//isfstream Stream2("/data/game/stk-code/data/po/zh_TW.po", "rb");
SDLRWopsifstream Stream2(SDL_RWFromFile("/data/game/stk-code/data/po/zh_TW.po", "rb"));
	std::string line;
	while (!safeGetline(Stream2, line).eof())
	{
		printf("%s\n",line.c_str());
	}

}
