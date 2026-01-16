#include <gtest/gtest.h>
#include "../../../sdv_services/core/toml_parser/character_reader_utf_8.h"
#include "../../../sdv_services/core/toml_parser/exception.h"

/* Requirements CharacterReader
 *
 * - UTF-8 conform
 *  + InputValidation
 *      + Recognize invalid bytes
 *      + Recognize invalid sequences
 *
 * - Peek() returns the next character without advancing the read location
 *  + PeekAndConsume_ReadCharacters
 *  + Peek_NoAdvance
 *
 * - Consume() returns the next character and advances the read location to the next character
 *  + PeekAndConsume_ReadCharacters
 *  + Consume_Advance
 *
 * - Peek(n) returns the next n-th character without advancing the read location; returns an empty string for n <= 0
 *  + PeekAndConsume_ReadCharacters
 *  + Peek_NoAdvance
 *
 * - Consume(n) returns the next n-th character and advances the read location to the next n-th character; returns an
 *      empty string and does not advance the read location for n <= 0; if Consume(n) would read after EOF it returns an empty
 *      string and advances the read location to the end
 *  + PeekAndConsume_ReadCharacters
 *  + Consume_Advance
 *
 * - PeekUntil(a) returns the next accumulation of characters until a character matches one of the characters given as parameter a,
 *      excluding the found character, without advancing the read location
 *  + PeekAndConsume_ReadCharacters
 *  + Peek_NoAdvance
 *  + PeekUntilConsumeUntil_FindAny
 *
 * - ConsumeUntil(a) returns the next accumulation of characters until a character matches one of the characters given as parameter
 *      a, excluding the found character, and advances the read location to the position of the triggering character
 *  + PeekAndConsume_ReadCharacters
 *  + Consume_Advance
 *  + PeekUntilConsumeUntil_FindAny
 *
 * - Peek() returns the same as Consume() at any given read location (attention to call-order when testing, since Consume()
 *      advances the read location!)
 *  + PeekAndConsume_SameOutput
 *
 * - Peek(n) returns the same as Consume(n) at any given read location for any given integer n (attention to call-order when
 *      testing, since Consume() advances the read location!)
 *  + PeekAndConsume_SameOutput
 *
 * - PeekUntil(a) returns the same as ConsumeUntil(a) at any given read location for any given Codepoint-collection a
 *  + PeekAndConsume_SameOutput
 *
 * - Peek()/Peek(n)/PeekUntil() and Consume()/Consume(n)/ConsumeUntil() have to detect and return any UTF-8 conform character
 *  + PeekAndConsume_ReadCharacters
 *
 * - If Consume(n)/ConsumeUntil(a) and Peek(n)/PeekUntil(a) would read over EOF they only read until EOF
 *  + PeekAndConsume_ReadCharacters
 *
 * - EOF() returns only true after a Consume() read the last character or would read out of bounds,
 *      or the given input is an empty string
 *  + EOFTest
 *
 * - The reader checks the input for UTF-8 validity on construction; if an invalid byte is found an XInvalidByteException is thrown;
 *      if an invalid Sequence is found, an XInvalidCharacterException is thrown
 *  + InputValidation
 */

std::string ASCII_LowerBound = std::string("") + static_cast<char>(0);
std::string ASCII_Between	 = "A";
std::string ASCII_UpperBound = std::string("") + static_cast<char>(127);

char FollowByte_LowerBound	= static_cast<char>(128); // 0x80
char FollowByte_Between		= static_cast<char>(150);
char FollowByte_UpperBound	= static_cast<char>(191); // 0xBF
char StartByte2B_LowerBound = static_cast<char>(194); // 0xC2
char StartByte2B_Between	= static_cast<char>(205);
char StartByte2B_UpperBound = static_cast<char>(223); // 0xDF
char StartByte3B_LowerBound = static_cast<char>(224); // 0xE0
char StartByte3B_Between	= static_cast<char>(231);
char StartByte3B_UpperBound = static_cast<char>(239); // 0xEF
char StartByte4B_LowerBound = static_cast<char>(240); // 0xF0
char StartByte4B_Between	= static_cast<char>(242);
char StartByte4B_UpperBound = static_cast<char>(244); // 0xF4

std::string TwoByteSequence_Low	   = std::string("") + StartByte2B_LowerBound + FollowByte_LowerBound;
std::string TwoByteSequence_Middle = std::string("") + StartByte2B_Between + FollowByte_LowerBound;
std::string TwoByteSequence_High   = std::string("") + StartByte2B_UpperBound + FollowByte_LowerBound;

std::string ThreeByteSequence_Low	 = std::string("") + StartByte3B_LowerBound + FollowByte_Between + FollowByte_UpperBound;
std::string ThreeByteSequence_Middle = std::string("") + StartByte3B_Between + FollowByte_Between + FollowByte_UpperBound;
std::string ThreeByteSequence_High	 = std::string("") + StartByte3B_UpperBound + FollowByte_Between + FollowByte_UpperBound;

std::string FourByteSequence_Low =
	std::string("") + StartByte4B_LowerBound + FollowByte_UpperBound + FollowByte_Between + FollowByte_LowerBound;
std::string FourByteSequence_Middle =
	std::string("") + StartByte4B_Between + FollowByte_UpperBound + FollowByte_Between + FollowByte_LowerBound;
std::string FourByteSequence_High =
	std::string("") + StartByte4B_UpperBound + FollowByte_UpperBound + FollowByte_Between + FollowByte_LowerBound;

std::string TestInputUTF8 = ASCII_LowerBound + TwoByteSequence_Low + ThreeByteSequence_Low + FourByteSequence_Low + ASCII_Between
							+ TwoByteSequence_Middle + ThreeByteSequence_Middle + FourByteSequence_Middle + ASCII_UpperBound
							+ TwoByteSequence_High + ThreeByteSequence_High + FourByteSequence_High;

std::map<std::size_t, std::string> TestInputUTF8_CharacterIndexMap = {{0, ASCII_LowerBound},
																	  {1, TwoByteSequence_Low},
																	  {2, ThreeByteSequence_Low},
																	  {3, FourByteSequence_Low},
																	  {4, ASCII_Between},
																	  {5, TwoByteSequence_Middle},
																	  {6, ThreeByteSequence_Middle},
																	  {7, FourByteSequence_Middle},
																	  {8, ASCII_UpperBound},
																	  {9, TwoByteSequence_High},
																	  {10, ThreeByteSequence_High},
																	  {11, FourByteSequence_High}};

std::string GreekAlphaUTF8		= std::string("") + static_cast<char>(0xCE) + static_cast<char>(0x91);
std::string PartialDifferential = std::string("") + static_cast<char>(0xE2) + static_cast<char>(0x88) + static_cast<char>(0x80);
std::string BraillePattern678	= std::string("") + static_cast<char>(0xE2) + static_cast<char>(0xA3) + static_cast<char>(0xA0);
std::string HiragenaNo			= std::string("") + static_cast<char>(0xE3) + static_cast<char>(0x81) + static_cast<char>(0xae);
std::string EmojiParachute =
	std::string("") + static_cast<char>(0xF0) + static_cast<char>(0x9F) + static_cast<char>(0xAA) + static_cast<char>(0x82);

std::string UTF8InputValid = "This is a valid UTF-8 input with:\n\t- Greek Alpha: " + GreekAlphaUTF8
							 + "\n\t- a partial differential: " + PartialDifferential
							 + "\n\t- a braille pattern: " + BraillePattern678 + "\n\t- the japanese hiragana 'no': " + HiragenaNo
							 + "\n\t- a parachute emoji: " + EmojiParachute + "\n";

auto GetInvalidUTF8Bytes = []() -> std::vector<std::string>
{
	std::vector<std::string> InvalidUTF8Bytes;
	InvalidUTF8Bytes.push_back(std::string("") + static_cast<char>(0xC0));
	InvalidUTF8Bytes.push_back(std::string("") + static_cast<char>(0xC1));
	InvalidUTF8Bytes.push_back(std::string("") + static_cast<char>(0xF5));
	InvalidUTF8Bytes.push_back(std::string("") + static_cast<char>(0xF6));
	InvalidUTF8Bytes.push_back(std::string("") + static_cast<char>(0xF7));
	InvalidUTF8Bytes.push_back(std::string("") + static_cast<char>(0xF8));
	InvalidUTF8Bytes.push_back(std::string("") + static_cast<char>(0xF9));
	InvalidUTF8Bytes.push_back(std::string("") + static_cast<char>(0xFA));
	InvalidUTF8Bytes.push_back(std::string("") + static_cast<char>(0xFB));
	InvalidUTF8Bytes.push_back(std::string("") + static_cast<char>(0xFC));
	InvalidUTF8Bytes.push_back(std::string("") + static_cast<char>(0xFD));
	InvalidUTF8Bytes.push_back(std::string("") + static_cast<char>(0xFE));
	InvalidUTF8Bytes.push_back(std::string("") + static_cast<char>(0xFF));

	return InvalidUTF8Bytes;
};

auto GetInvalidUTF8Sequences = []() -> std::vector<std::string>
{
	std::vector<std::string> InvalidUTF8Sequences;
	InvalidUTF8Sequences.push_back(std::string("") + FollowByte_LowerBound);
	InvalidUTF8Sequences.push_back(std::string("") + FollowByte_Between);
	InvalidUTF8Sequences.push_back(std::string("") + FollowByte_UpperBound);
	InvalidUTF8Sequences.push_back(std::string("") + StartByte2B_LowerBound + StartByte2B_LowerBound);
	InvalidUTF8Sequences.push_back(std::string("") + StartByte2B_Between + StartByte3B_Between);
	InvalidUTF8Sequences.push_back(std::string("") + StartByte2B_UpperBound + StartByte4B_UpperBound);
	InvalidUTF8Sequences.push_back(std::string("") + StartByte2B_UpperBound + ASCII_Between);
	InvalidUTF8Sequences.push_back(std::string("") + StartByte3B_LowerBound + FollowByte_Between + StartByte2B_Between);
	InvalidUTF8Sequences.push_back(std::string("") + StartByte3B_Between + FollowByte_Between + StartByte3B_UpperBound);
	InvalidUTF8Sequences.push_back(std::string("") + StartByte3B_UpperBound + FollowByte_Between + StartByte4B_LowerBound);
	InvalidUTF8Sequences.push_back(std::string("") + StartByte3B_UpperBound + FollowByte_Between + ASCII_Between);
	InvalidUTF8Sequences.push_back(std::string("") + StartByte4B_LowerBound + FollowByte_Between + FollowByte_Between
								   + StartByte2B_UpperBound);
	InvalidUTF8Sequences.push_back(std::string("") + StartByte4B_Between + FollowByte_Between + FollowByte_Between
								   + StartByte3B_LowerBound);
	InvalidUTF8Sequences.push_back(std::string("") + StartByte4B_UpperBound + FollowByte_Between + FollowByte_Between
								   + StartByte4B_Between);
	InvalidUTF8Sequences.push_back(std::string("") + StartByte4B_UpperBound + FollowByte_Between + FollowByte_Between
								   + ASCII_Between);
	InvalidUTF8Sequences.push_back(std::string("") + StartByte2B_Between + FollowByte_Between + FollowByte_Between
								   + FollowByte_Between);
	InvalidUTF8Sequences.push_back(std::string("") + StartByte3B_Between + FollowByte_Between + FollowByte_Between
								   + FollowByte_Between + FollowByte_Between);
	InvalidUTF8Sequences.push_back(std::string("") + StartByte4B_Between + FollowByte_Between + FollowByte_Between
								   + FollowByte_Between + FollowByte_Between + FollowByte_Between);
	return InvalidUTF8Sequences;
};

TEST(UTF8_CharacterReader, InputValidation)
{
	// Test for every infalid byte in UTF-8 that it will be caught as invalid input
	for (const auto& inv : GetInvalidUTF8Bytes())
	{
		std::string ssUTF8InputInvalid = "Invalid input: " + inv + " test";
		EXPECT_THROW(toml_parser::CCharacterReaderUTF8 reader(ssUTF8InputInvalid), sdv::toml::XTOMLParseException);
	}
	// Test for a variety of invalid sequences
	for (const auto& inv : GetInvalidUTF8Sequences())
	{
		std::string UTF8InputInvalid = "Invalid input: " + inv + "test";
		EXPECT_THROW(toml_parser::CCharacterReaderUTF8 reader(UTF8InputInvalid), sdv::toml::XTOMLParseException);
	}
	// Test for a sample of ASCII, 2-Byte-, 3-Byte- and 4-Byte-Sequences that they will be accepted as valid input
	EXPECT_NO_THROW(toml_parser::CCharacterReaderUTF8 reader(UTF8InputValid));

	EXPECT_NO_THROW(toml_parser::CCharacterReaderUTF8 reader(TestInputUTF8));
}

TEST(UTF8_CharacterReader, PeekAndConsume_ReadCharacters)
{
    // Peek() and Consume() read the next character
    {
        toml_parser::CCharacterReaderUTF8 reader(TestInputUTF8);
        for (int i = 0; i < 12; ++i)
        {
            EXPECT_EQ(TestInputUTF8_CharacterIndexMap[i], reader.Peek());
            EXPECT_EQ(TestInputUTF8_CharacterIndexMap[i], reader.Consume());
        }
    }

	// Read all characters and the an empty string must be read
    {
        toml_parser::CCharacterReaderUTF8 reader(TestInputUTF8);
        std::size_t nSize = TestInputUTF8.size();
		size_t nCnt = 0;
        while (nCnt < nSize)
        {
            EXPECT_FALSE(reader.Peek().empty());
            std::string ssCharacters = reader.Consume();
			size_t nCharCnt = ssCharacters.size();
            EXPECT_NE(nCharCnt, 0);
            if (!nCharCnt) break;
            nCnt += nCharCnt;
        }
        EXPECT_EQ(nCnt, nSize);
        EXPECT_TRUE(reader.Peek().empty());
        EXPECT_TRUE(reader.Consume().empty());
    }
}

TEST(UTF8_CharacterReader, PeekAndConsume_ReadCharactersSkip)
{
    // Peek(n) and Consume(n) read with skip of every two characters
	{
		toml_parser::CCharacterReaderUTF8 reader(TestInputUTF8);
		for (int i = 1; i < 12; i += 2)
		{

			EXPECT_EQ(TestInputUTF8_CharacterIndexMap[i], reader.Peek(1));
			EXPECT_EQ(TestInputUTF8_CharacterIndexMap[i], reader.Consume(1));
		}
	}

	// Peek(0)/Consume(0) will return the first character in the string and Peek(n)/Consume(n) return empty string if they read out
    // of bounds
    {
        toml_parser::CCharacterReaderUTF8 reader(TestInputUTF8);
        EXPECT_EQ(TestInputUTF8_CharacterIndexMap[0], reader.Peek(0));
        EXPECT_EQ(TestInputUTF8_CharacterIndexMap[0], reader.Consume(0));
        std::size_t biggerIndex = TestInputUTF8.size(); // Assure this is bigger than the number of characters in TestInputUTF8
        EXPECT_EQ("", reader.Peek(biggerIndex));
        EXPECT_EQ("", reader.Consume(biggerIndex));
    }
}

TEST(UTF8_CharacterReader, PeekAndConsume_ReadCharactersMulti)
{
    // Peek() and Consume() read the next character
    {
        toml_parser::CCharacterReaderUTF8 reader(TestInputUTF8);
        for (int i = 0; i < 12; i += 2)
        {
            EXPECT_EQ(TestInputUTF8_CharacterIndexMap[i] + TestInputUTF8_CharacterIndexMap[i + 1], reader.Peek(0, 2));
            EXPECT_EQ(TestInputUTF8_CharacterIndexMap[i] + TestInputUTF8_CharacterIndexMap[i + 1], reader.Consume(0, 2));
        }
    }

    // Peek() and Consume() skip one and then read two read the next character
    {
        toml_parser::CCharacterReaderUTF8 reader(TestInputUTF8);
        for (int i = 0; i < 12; i += 3)
        {
            EXPECT_EQ(TestInputUTF8_CharacterIndexMap[i + 1] + TestInputUTF8_CharacterIndexMap[i + 2], reader.Peek(1, 2));
            EXPECT_EQ(TestInputUTF8_CharacterIndexMap[i + 1] + TestInputUTF8_CharacterIndexMap[i + 2], reader.Consume(1, 2));
        }
    }
}

TEST(UTF8_CharacterReader, PeekAndConsume_ReadCharactersUntil)
{
    // PeekUntil(a) and ConsumeUntil(a) read until a given character
	{
		std::size_t byteIndex = 0;
		for (int i = 0; i < 12; ++i)
		{
			toml_parser::CCharacterReaderUTF8		 reader(TestInputUTF8);
			std::vector<std::string> Codepoints;
			Codepoints.push_back(TestInputUTF8_CharacterIndexMap[i]);
			std::string wantedSubstring = TestInputUTF8.substr(0, byteIndex);
			EXPECT_EQ(wantedSubstring, reader.PeekUntil(Codepoints));
			EXPECT_EQ(wantedSubstring, reader.ConsumeUntil(Codepoints));
			byteIndex += TestInputUTF8_CharacterIndexMap[i].size();
		}
	}
	// PeekUntil(a) and ConsumeUntil(a) read only  until EOF if they don't find a matching character
	{
		toml_parser::CCharacterReaderUTF8 reader(TestInputUTF8);
		std::string			notInTestString = "g";
		EXPECT_EQ(std::string::npos, TestInputUTF8.find(notInTestString));
		EXPECT_EQ(TestInputUTF8, reader.PeekUntil({notInTestString}));
		EXPECT_EQ(TestInputUTF8, reader.ConsumeUntil({notInTestString}));
	}
}

TEST(UTF8_CharacterReader, Peek_NoAdvance)
{
	ASSERT_NE(TestInputUTF8_CharacterIndexMap[0], TestInputUTF8_CharacterIndexMap[1]);
	ASSERT_NE(TestInputUTF8_CharacterIndexMap[1], TestInputUTF8_CharacterIndexMap[2]);
	toml_parser::CCharacterReaderUTF8 reader(TestInputUTF8);
	// Peek() does not advance the read location
	{
		EXPECT_EQ(TestInputUTF8_CharacterIndexMap[0], reader.Peek());
		EXPECT_EQ(TestInputUTF8_CharacterIndexMap[0], reader.Peek());
		EXPECT_EQ(TestInputUTF8_CharacterIndexMap[0], reader.Peek());
	}
	// Peek(n) does not advance the read location
	{
        EXPECT_EQ(TestInputUTF8_CharacterIndexMap[0], reader.Peek(0));		// Read pos 0
		EXPECT_EQ(TestInputUTF8_CharacterIndexMap[1], reader.Peek(1));		// Read pos 1
		EXPECT_EQ(TestInputUTF8_CharacterIndexMap[2], reader.Peek(2));		// Read pos 2
		EXPECT_EQ(TestInputUTF8_CharacterIndexMap[3], reader.Peek(3));		// Read pos 3
	}
	// PeekUntil(a) does not advance the read location
	{
		EXPECT_EQ(TestInputUTF8_CharacterIndexMap[0], reader.PeekUntil({TestInputUTF8_CharacterIndexMap[1]}));
		EXPECT_EQ(TestInputUTF8_CharacterIndexMap[0] + TestInputUTF8_CharacterIndexMap[1],
				  reader.PeekUntil({TestInputUTF8_CharacterIndexMap[2]}));
		EXPECT_EQ(TestInputUTF8_CharacterIndexMap[0] + TestInputUTF8_CharacterIndexMap[1] + TestInputUTF8_CharacterIndexMap[2],
				  reader.PeekUntil({TestInputUTF8_CharacterIndexMap[3]}));
	}
}

TEST(UTF8_CharacterReader, Consume_Advance)
{
	ASSERT_NE(TestInputUTF8_CharacterIndexMap[0], TestInputUTF8_CharacterIndexMap[1]);
	ASSERT_NE(TestInputUTF8_CharacterIndexMap[1], TestInputUTF8_CharacterIndexMap[2]);
	ASSERT_NE(TestInputUTF8_CharacterIndexMap[2], TestInputUTF8_CharacterIndexMap[3]);
	ASSERT_NE(TestInputUTF8_CharacterIndexMap[3], TestInputUTF8_CharacterIndexMap[4]);
	// Consume() does advance the read location to the next character
	{
		toml_parser::CCharacterReaderUTF8 reader(TestInputUTF8);
		EXPECT_EQ(TestInputUTF8_CharacterIndexMap[0], reader.Consume());
		EXPECT_EQ(TestInputUTF8_CharacterIndexMap[1], reader.Consume());
		EXPECT_EQ(TestInputUTF8_CharacterIndexMap[2], reader.Consume());
	}
	// Consume(n) does advance the read location to the next n-th character
	{
		toml_parser::CCharacterReaderUTF8 reader(TestInputUTF8);
        EXPECT_EQ(TestInputUTF8_CharacterIndexMap[0], reader.Consume(0));	// Read pos 0; advance to pos 1
		EXPECT_EQ(TestInputUTF8_CharacterIndexMap[2], reader.Consume(1));	// Read pos 2; advance to pos 3
		EXPECT_EQ(TestInputUTF8_CharacterIndexMap[5], reader.Consume(2));	// Read pos 5; advance to pos 6
		EXPECT_EQ(TestInputUTF8_CharacterIndexMap[9], reader.Consume(3));	// Read pos 9; advance to pos 10
	}
	// ConsumeUntil(a) does advance the read location
	{
		toml_parser::CCharacterReaderUTF8 reader(TestInputUTF8);
		EXPECT_EQ(TestInputUTF8_CharacterIndexMap[0], reader.ConsumeUntil({TestInputUTF8_CharacterIndexMap[1]}));
		EXPECT_EQ(TestInputUTF8_CharacterIndexMap[1], reader.ConsumeUntil({TestInputUTF8_CharacterIndexMap[2]}));
		EXPECT_EQ(TestInputUTF8_CharacterIndexMap[2], reader.ConsumeUntil({TestInputUTF8_CharacterIndexMap[3]}));
	}
}

TEST(UTF8_CharacterReader, PeekUntilConsumeUntil_FindAny)
{
	// PeekUntil(a) works for a collection of characters
	{
		toml_parser::CCharacterReaderUTF8 reader(TestInputUTF8);
		EXPECT_EQ(TestInputUTF8_CharacterIndexMap[0],
				  reader.PeekUntil({TestInputUTF8_CharacterIndexMap[2], TestInputUTF8_CharacterIndexMap[1]}));
		EXPECT_EQ(TestInputUTF8_CharacterIndexMap[0] + TestInputUTF8_CharacterIndexMap[1] + TestInputUTF8_CharacterIndexMap[2],
				  reader.PeekUntil({TestInputUTF8_CharacterIndexMap[3], TestInputUTF8_CharacterIndexMap[4]}));
	}
	// ConsumeUntil(a) works for a collection of characters
	{
		toml_parser::CCharacterReaderUTF8 reader(TestInputUTF8);
		EXPECT_EQ(TestInputUTF8_CharacterIndexMap[0],
				  reader.ConsumeUntil({TestInputUTF8_CharacterIndexMap[2], TestInputUTF8_CharacterIndexMap[1]}));
		EXPECT_EQ(TestInputUTF8_CharacterIndexMap[1] + TestInputUTF8_CharacterIndexMap[2],
				  reader.ConsumeUntil({TestInputUTF8_CharacterIndexMap[3], TestInputUTF8_CharacterIndexMap[4]}));
	}
}

TEST(UTF8_CharacterReader, PeekAndConsume_SameOutput)
{
	{
		toml_parser::CCharacterReaderUTF8 reader(TestInputUTF8);
		for (int i = 0; i < 12; ++i)
		{
			std::string p = reader.Peek();
			std::string c = reader.Consume();
			EXPECT_EQ(p, c);
		}
	}
	{
		for (int i = 1; i < 13; ++i)
		{
			toml_parser::CCharacterReaderUTF8 reader(TestInputUTF8);
			std::string			p = reader.Peek(i);
			std::string			c = reader.Consume(i);
			EXPECT_EQ(p, c);
		}
	}
	{
		for (int i = 1; i < 13; ++i)
		{
			toml_parser::CCharacterReaderUTF8		 reader(TestInputUTF8);
			std::vector<std::string> CodePoints;
			CodePoints.push_back(reader.Peek(i));
			std::string p = reader.PeekUntil(CodePoints);
			std::string c = reader.ConsumeUntil(CodePoints);
			EXPECT_EQ(p, c);
		}
	}
}

TEST(UTF8_CharacterReader, EOFTests)
{
	// Check all calls that are not to trigger EOF
	{
		toml_parser::CCharacterReaderUTF8 reader(TestInputUTF8);
		EXPECT_FALSE(reader.IsEOF());
		reader.Peek();
		reader.Peek(1);
		reader.Peek(TestInputUTF8_CharacterIndexMap.size());
		reader.Peek(TestInputUTF8_CharacterIndexMap.size() + 1);
		reader.PeekUntil({"g"});
		reader.Consume();
		reader.Consume(1);
		reader.ConsumeUntil({TestInputUTF8_CharacterIndexMap[TestInputUTF8_CharacterIndexMap.size() - 3]});
		EXPECT_FALSE(reader.IsEOF());
	}
	// Check that reading the last character with Consume(n) triggers EOF
	{
		toml_parser::CCharacterReaderUTF8 reader(TestInputUTF8);
		EXPECT_FALSE(reader.IsEOF());
		reader.Consume(TestInputUTF8_CharacterIndexMap.size()); // Last Character
		EXPECT_TRUE(reader.IsEOF());
	}
	// Check that reading out of bounds with Consume(n) triggers EOF
	{
		toml_parser::CCharacterReaderUTF8 reader(TestInputUTF8);
		EXPECT_FALSE(reader.IsEOF());
		reader.Consume(TestInputUTF8_CharacterIndexMap.size() + 1); // Out of bounds
		EXPECT_TRUE(reader.IsEOF());
	}
	{
		toml_parser::CCharacterReaderUTF8 reader(TestInputUTF8);
		EXPECT_FALSE(reader.IsEOF());
		reader.ConsumeUntil({TestInputUTF8_CharacterIndexMap[11]}); // Second last character
		EXPECT_FALSE(reader.IsEOF());
		reader.ConsumeUntil({"g"}); // Character not in reader; consumes until the end
		EXPECT_TRUE(reader.IsEOF());
	}
	{
		toml_parser::CCharacterReaderUTF8 reader("");
		EXPECT_TRUE(reader.IsEOF());
	}
}
