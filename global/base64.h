#ifndef BASE64_H
#define BASE64_H

#include <string>
#include <stdexcept>

static const std::string Base64Chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

/**
 * @brief check whether its a Base64 format
 * @param[in] c character to be looked for
 * @return true if Base64 format
 */
inline bool IsBase64(const unsigned char c)
{
    return (isalnum(c) || (c == '+') || (c == '/'));
}

/**
 * @brief Encode to Base64 format
 * @param[in] plainText text to be encoded to Base64
 * @return text encoded to format
 */
inline std::string Base64EncodePlainText(const std::string& plainText)
{
    const char* buffer = plainText.c_str();
    std::size_t bytesLeft = plainText.size();

    std::string ret;
    int i = 0;
    unsigned char char_array_3[3]{ 0 };
    unsigned char char_array_4[4]{ 0 };

    while (bytesLeft--)
    {
        char_array_3[i++] = *reinterpret_cast<const unsigned char*>((buffer++));
        if (i == 3)
        {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; (i < 4); i++)
            {
                ret += Base64Chars[char_array_4[i]];
            }
            i = 0;
        }
    }

    if (i)
    {
        for (int j = i; j < 3; j++)
        {
            char_array_3[j] = '\0';
        }

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (int j = 0; (j < i + 1); j++)
        {
            ret += Base64Chars[char_array_4[j]];
        }

        while ((i++ < 3))
        {
            ret += '=';
        }
    }

    return ret;
}


/**
 * @brief Decode Base64 formatstring
 * @param[in] encoded_string to be decoded
 * @return decoded string
 */
inline std::string Base64DecodePlainText(const std::string& encoded_string)
{
    size_t in_len = encoded_string.size();
    size_t i = 0;
    size_t in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && (encoded_string[in_] != '=') && IsBase64(encoded_string[in_]))
    {
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i == 4)
        {
            for (i = 0; i < 4; i++)
            {
                char_array_4[i] = static_cast<uint8_t>(Base64Chars.find(char_array_4[i]));
            }

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
            {
                ret+=(reinterpret_cast<char*>(char_array_3)[i]);
            }
            i = 0;
        }
    }

    if (i)
    {
        for (size_t j = i; j < 4; j++)
        {
            char_array_4[j] = 0;
        }

        for (size_t j = 0; j < 4; j++)
        {
            char_array_4[j] = static_cast<uint8_t>(Base64Chars.find(char_array_4[j]));
        }

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (size_t j = 0; (j < i - 1); j++)
        {
            ret += (reinterpret_cast<char*>(char_array_3)[j]);
        }
    }
    return ret;
}

/**
 * @brief Encode to Base64 format
 * @param[in] data buffer to be encoded to Base64
 * @param[in] datalength buffer length to be encoded to Base64
 * @return text encoded to format
 */
inline std::string Base64Encode(const char* data, std::size_t datalength)
{
    const char* buffer = data;
    std::size_t bytesLeft = datalength;

    std::string ret;
    int i = 0;
    unsigned char char_array_3[3]{ 0 };
    unsigned char char_array_4[4]{ 0 };

    while (bytesLeft--)
    {
        char_array_3[i++] = *(buffer++);
        if (i == 3)
        {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; (i < 4); i++)
            {
                ret += Base64Chars[char_array_4[i]];
            }
            i = 0;
        }
    }

    if (i)
    {
        for (int j = i; j < 3; j++)
        {
            char_array_3[j] = '\0';
        }

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (int j = 0; (j < i + 1); j++)
        {
            ret += Base64Chars[char_array_4[j]];
        }

        while ((i++ < 3))
        {
            ret += '=';
        }
    }

    return ret;
}

/**
 * @brief Decode Base64 buffer
 * @param[in] data buffer to be encoded to Base64
 * @return decoded string
 */
template <class TConvert>
inline std::string Base64Encode(const TConvert& data)
{
    return Base64Encode(reinterpret_cast<const char*>(&data), sizeof(data));
}

/**
 * @brief Decode Base64 formatstring
 * @param[in] encoded_string to be decoded
 * @return decoded string
 */
inline std::vector<unsigned char> Base64Decode(const std::string& encoded_string)
{
    size_t in_len = encoded_string.size();
    size_t i = 0;
    size_t in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::vector<unsigned char> ret;

    while (in_len-- && (encoded_string[in_] != '=') && IsBase64(encoded_string[in_]))
    {
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i == 4)
        {
            for (i = 0; i < 4; i++)
            {
                char_array_4[i] = static_cast<uint8_t>(Base64Chars.find(char_array_4[i]));
            }

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
            {
                ret.push_back(char_array_3[i]);
            }
            i = 0;
        }
    }

    if (i)
    {
        for (size_t j = i; j < 4; j++)
        {
            char_array_4[j] = 0;
        }

        for (size_t j = 0; j < 4; j++)
        {
            char_array_4[j] = static_cast<uint8_t>(Base64Chars.find(char_array_4[j]));
        }

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (size_t j = 0; (j < i - 1); j++)
        {
            ret.push_back(char_array_3[j]);
        }
    }
    return ret;
    // memcpy_s(&datablock, sizeof(datablock), ret.data(), ret.size());
}

/**
 * @brief Decode Base64 formatstring
 * @param[in] encoded_string to be decoded
 * @return decoded string
 */
template <class TConvert>
inline TConvert DecodeBase64(const std::string& encoded_string)
{
    TConvert out{};
    std::vector<unsigned char> rawData = Base64Decode(encoded_string);
    if (sizeof(out) == rawData.size())
    {
        memcpy_s(&out, sizeof(out), rawData.data(), rawData.size());
    }
    else
    {
        TConvert invalid{};
        return invalid;
    }
    return out;
}

/**
 * @brief Decode Base64 formatstring
 * @param[in] encoded_string to be decoded
 * @return decoded string
 */
template <>
inline std::string DecodeBase64(const std::string& encoded_string)
{
    std::vector<unsigned char> rawData = Base64Decode(encoded_string);
    std::string out(reinterpret_cast<char*>(rawData.data()), rawData.size());
    return out;
}
#endif
