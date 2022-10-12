#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#ifndef _DEBUG
#include <svdpi.h>
#endif // _DEBUG

class Text2D {
  public:
    explicit Text2D (const std::string filename, const std::ios::openmode openmode,
                     const unsigned int bitdepth, const std::string delimiter, const int padding) :
        m_fs(filename, openmode), m_openmode(openmode), m_fpos(m_fs.tellg()),
        m_cache(0), m_cache_width(0), m_cache_height(0),
        m_bitdepth(bitdepth), m_delimiter(delimiter), m_padding(padding) {
        ;
    }

    virtual ~Text2D() { ; }

    static Text2D *create(const std::string filename, const std::string flags,
                          const unsigned int bitdepth, const std::string delimiter, const int padding) {
        std::ios::openmode openmode;

        if (flags == "r") {
            openmode = std::ios::in;
        } else if (flags == "w") {
            openmode = std::ios::out;
        } else {
            return nullptr;
        }

        return new Text2D(filename, openmode, bitdepth, delimiter, padding);
    }

    static void destroy(Text2D *instance) {
        if (instance != nullptr) {
            delete instance;
        }
    }

    void cache_clear() {
        std::fill(m_cache.begin(), m_cache.end(), 0);
    }

    void cache_resize(const unsigned int width, const unsigned int height) {
        m_cache.resize(width * height);
        m_cache_width = width;
        m_cache_height = height;
    }

    void cache_dump(int dump_header) {
        if (dump_header > 0) {
            std::cout << "-- cache_dump() called -- " << std::endl;
            std::cout << "     ";
            for (unsigned int x = 0; x < m_cache_width; ++x) {
                std::cout << std::right << std::setw(2) << x << " ";
            }
            std::cout << std::endl;
            std::cout << "    ";
            for (unsigned int x = 0; x < m_cache_width; ++x) {
                std::cout << "---";
            }
            std::cout << std::endl;
        }

        for (unsigned int y = 0; y < m_cache_height; ++y) {
            if (dump_header > 0) {
                std::cout << std::right << std::setfill(' ') << std::setw(2) << std::dec << y << " | ";
            }
            for (unsigned int x = 0; x < m_cache_width; ++x) {
                std::cout << std::right << std::setfill('0') << std::setw(2) << std::hex <<
                    static_cast<int>(m_cache[m_cache_width * y + x]) << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::setfill(' ');
    }

    int file_read(const unsigned int lines) {
        auto ret = (!m_fs) ? -1 : 0;

        if (ret == 0) {
            ret = (m_openmode != std::ios::in) ? -1 : 0;
        }

        if (ret == 0) {
            m_cache.clear();

            try {
                m_fs.seekg(m_fpos, std::ios::beg);

                auto clear_cache_height = true;
                std::string line;
                while (std::getline(m_fs >> std::ws, line)) {
                    std::istringstream istr(line);
                    std::string data;

                    if ((line[0] != '/') &&
                        (line[0] != '#')) {
                        auto clear_cache_width = true;
                        while (std::getline(istr >> std::ws, data, m_delimiter.c_str()[0])) {
                            m_cache.push_back(std::stoi(data, nullptr, 16));

                            if (clear_cache_width) {
                                m_cache_width = 0;
                                clear_cache_width = false;
                            }
                            ++m_cache_width;
                        }

                        if (clear_cache_height) {
                            m_cache_height = 0;
                            clear_cache_height = false;
                        }
                        ++m_cache_height;

                        if (m_cache_height >= lines) {
                            m_fpos = m_fs.tellg();
                            break;
                        }
                    }
                }
            } catch (std::invalid_argument &e) {
                std::cerr << "invalid_argument exception occured !!! [" << e.what() << "]" << std::endl;
                ret = -1;
            } catch (std::out_of_range &e) {
                std::cerr << "out_of_range exception occured !!! [" << e.what() << "]" << std::endl;
                ret = -1;
            } catch (...) {
                std::cerr << "unknown exception occured !!!" << std::endl;
                ret = -1;
            }
        }

        return ret;
    }

    int file_write() {
        auto ret = (!m_fs) ? -1 : 0;

        if (ret == 0) {
            ret = (m_openmode != std::ios::out) ? -1 : 0;
        }

        if (ret == 0) {
            for (unsigned int y = 0; y < m_cache_height; ++y) {
                for (unsigned int x = 0; x < (m_cache_width - 1); ++x) {
                    m_fs << std::right << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(m_cache[m_cache_width * y + x]) << "_";
                }
                m_fs << std::right << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(m_cache[m_cache_width * y + (m_cache_width - 1)]) << std::endl;
            }
        }

        return ret;
    }

    int cache_read(void *data, const int cachex, const int cachey, const unsigned int read_width, const unsigned int read_height,
                   const unsigned int stridex = 1, const unsigned int stridey = 1) {
        auto *d = static_cast<uint8_t *>(data);
        auto ret = (d == nullptr) ? -1 : 0;

        if (ret == 0) {
            ret = ((stridex > 0) && (stridey > 0)) ? 0 : -1;
        }

        if (ret == 0) {
            auto cnt = 0;
            auto endx = cachex + static_cast<int>(read_width) * static_cast<int>(stridex);
            auto endy = cachey + static_cast<int>(read_height) * static_cast<int>(stridey);

            for (auto y = cachey; y < endy; y += stridey) {
                for (auto x = cachex; x < endx; x += stridex) {
                    if ((y < 0) || (y >= static_cast<int>(m_cache_height))) {
                        d[cnt] = m_padding;
                    } else if ((x < 0) || (x >= static_cast<int>(m_cache_width))) {
                        d[cnt] = m_padding;
                    } else {
                        d[cnt] = m_cache[m_cache_width * y + x];
                    }
                    ++cnt;
                }
            }
        }

        return ret;
    }

    int cache_write(void *data, const unsigned int cachex, const unsigned int cachey, const unsigned int write_width, const unsigned int write_height) {
        auto *d = static_cast<uint8_t *>(data);
        auto ret = (d == nullptr) ? -1 : 0;

        if (ret == 0) {
            auto cnt = 0;
            auto endx = cachex + write_width;
            auto endy = cachey + write_height;

            for (auto y = cachey; y < endy; ++y) {
                for (auto x = cachex; x < endx; ++x) {
                    if ((y < m_cache_height) && (x < m_cache_width)) {
                        m_cache[m_cache_width * y + x] = d[cnt];
                    }
                    ++cnt;
                }
            }
        }

        return ret;
    }

  protected:
    std::fstream m_fs;
    const std::ios::openmode m_openmode;
    std::streampos m_fpos;
    std::vector<uint8_t> m_cache;
    unsigned int m_cache_width;
    unsigned int m_cache_height;
    const unsigned int m_bitdepth;
    const std::string m_delimiter;
    const int m_padding;
};

#ifdef __cplusplus
extern "C"
{
#endif

void *text2d_open(const char *filename, const char *flags, const unsigned int bitdepth, const char *delimiter, const int padding)
{
    printf("delimiter: %s\n", delimiter);
    return static_cast<void *>(Text2D::create(filename, flags, bitdepth, delimiter, padding));
}

void text2d_close(void *handle)
{
    Text2D::destroy(static_cast<Text2D *>(handle));
}

void text2d_cache_clear(void *handle)
{
    static_cast<Text2D *>(handle)->cache_clear();
}

void text2d_cache_resize(void *handle, const unsigned int width, const unsigned int height)
{
    static_cast<Text2D *>(handle)->cache_resize(width, height);
}

void text2d_cache_dump(void *handle, const int dump_header)
{
    return static_cast<Text2D *>(handle)->cache_dump(dump_header);
}

int text2d_fread(void *handle, const unsigned int lines)
{
    return static_cast<Text2D *>(handle)->file_read(lines);
}

int text2d_fwrite(void *handle)
{
    return static_cast<Text2D *>(handle)->file_write();
}

#ifndef _DEBUG
int text2d_cache_read(void *handle, const svOpenArrayHandle array, const int cachex, const int cachey, const unsigned int read_width, const unsigned int read_height, const unsigned int stridex, const unsigned int stridey)
{
    return static_cast<Text2D *>(handle)->cache_read(svGetArrayPtr(array), cachex, cachey, read_width, read_height, stridex, stridey);
}

int text2d_cache_write(void *handle, const svOpenArrayHandle array, const int cachex, const int cachey, const unsigned int write_width, const unsigned int write_height)
{
    return static_cast<Text2D *>(handle)->cache_write(svGetArrayPtr(array), cachex, cachey, write_width, write_height);
}
#else // _DEBUG
int text2d_cache_read(void *handle, uint8_t *array, const int cachex, const int cachey, const unsigned int read_width, const unsigned int read_height, const unsigned int stridex, const unsigned int stridey)
{
    return static_cast<Text2D *>(handle)->cache_read(static_cast<void *>(array), cachex, cachey, read_width, read_height, stridex, stridey);
}

int text2d_cache_write(void *handle, uint8_t *array, const int cachex, const int cachey, const unsigned int write_width, const unsigned int write_height)
{
    return static_cast<Text2D *>(handle)->cache_write(static_cast<void *>(array), cachex, cachey, write_width, write_height);
}
#endif // _DEBUG

#ifdef __cplusplus
}
#endif

#ifdef _DEBUG

#include <random>

//----------------------------------
// c++ sample program
//----------------------------------
int main() {
    std::mt19937 mt(0);
    std::vector<uint8_t> arr(8 * 4);
    auto randomize = [&]() { for (auto &a : arr) { a = mt(); } };

    std::cout << "//----------------------------------" << std::endl;
    std::cout << "// write phase"                       << std::endl;
    std::cout << "//----------------------------------" << std::endl;

    Text2D *wtxt = (Text2D *)text2d_open("sample.txt", "w", 8, "_", 0);
    if (wtxt == nullptr) {
        std::cerr << "text2d_open() returns null pointer !!!" << std::endl;
    }

    text2d_cache_resize(wtxt, 32, 32);

    std::uniform_int_distribution<> dist(0, 16);

    for (auto c = 0; c < 2; ++c) {
        for (auto y = 0; y < 32; y += 4) {
            for (auto x = dist(mt); x < 32; x += 16) {
                randomize();
                text2d_cache_write(wtxt, arr.data(), x, y, 8, 4);
            }
        }
        text2d_cache_dump(wtxt, 1);

        if (text2d_fwrite(wtxt) < 0) {
            std::cerr << "text2d_fwrite() error !!!" << std::endl;
        }
        text2d_cache_clear(wtxt);
    }

    text2d_close(wtxt);

    std::cout << "//----------------------------------" << std::endl;
    std::cout << "// read phase"                        << std::endl;
    std::cout << "//----------------------------------" << std::endl;

    arr.resize(10 * 6);

    Text2D *rtxt = (Text2D *)text2d_open("sample.txt", "r", 8, "_", -1);
    if (rtxt == nullptr) {
        std::cerr << "text2d_open() returns null pointer !!!" << std::endl;
    }

    for (auto c = 0; c < 2; ++c) {
        text2d_fread(rtxt, 32);
        text2d_cache_dump(rtxt, 1);

        for (auto y = 0; y < 32; y += 4) {
            for (auto x = 0; x < 32; x += 8) {
                text2d_cache_read(rtxt, arr.data(), x - 1, y - 1, 10, 6, c + 1, c + 1);

                std::cout << std::dec << "-- (x, y) = (" << x << ", " << y << ")" << std::endl;
                for (auto y = 0; y < 6; ++y) {
                    for (auto x = 0; x < 10; ++x) {
                        std::cout << std::right << std::setfill('0') << std::setw(2) << std::hex <<
                            static_cast<int>(arr[y * 10 + x]) << " ";
                    }
                    std::cout << std::endl;
                }
            }
        }
    }

    text2d_close(rtxt);
}

#endif // _DEBUG

