#include <iostream>
#include <sstream>

#include "Bitstream.h"

BitStream::BitStream(std::ios & stream): m_stream(stream), m_paddingBitLength(CHAR_BIT), m_currentBitNum(1), m_currentByte(0)
{

}

BitStream::~BitStream()
{

}

OutputBitStream::OutputBitStream(std::ostream & stream): BitStream(stream)
{

}


OutputBitStream::~OutputBitStream()
{

}

void OutputBitStream::InsertBits(std::string bits)
{
	for (unsigned int i = 0 ; i < bits.length(); i++) {
        	char bitToAdd = bits[i];
        	this->InsertBit(bitToAdd);
    	}
}

void OutputBitStream::InsertBit(char bit)
{
    	if (bit != '0' && bit != '1')
		std::cerr << "Invalid Bit character given: " << bit <<  std::endl;

    	if (bit == '0')
		this->InsertBit((unsigned int)0);
    	else
        	this->InsertBit((unsigned int)1);

}

void OutputBitStream::InsertBit(unsigned int bit)
{
    bit &= 1; //sanity check to make sure it is only 0 or 1
    bit = bit << (CHAR_BIT - this->m_currentBitNum); //lets move the bit to the current position (if it's a one it matters onl)
    this->m_paddingBitLength = CHAR_BIT - this->m_currentBitNum;
    this->m_currentByte |= bit;

    this->m_currentBitNum += 1;

    if (this->m_currentBitNum > CHAR_BIT)
    {
        this->Flush();
    }

}

void OutputBitStream::Flush()
{
    if (this->m_currentBitNum > 1)//only append truly if we've process any bits
    {
        this->GetStream() << this->m_currentByte;
        this->GetStream().flush();
        this->m_currentByte = 0;
        this->m_currentBitNum = 1;
    }
}

std::ostream & operator<< (std::ostream & os, const OutputBitStream & bitStream)
{
    os << bitStream.m_stream.rdbuf();
    return os;
}


InputBitStream::InputBitStream(std::istream & stream): BitStream(stream)
{
    this->m_currentBitNum = CHAR_BIT ;
}

InputBitStream::~InputBitStream()
{

}

void InputBitStream::Consume()
{
    //Unsure why >> is different than get(). Something with my use of unsigned but I still don't understand.
    //>> was reading hex values of 0x0A as 0x6F... weird.
    //this->GetStream() >> this->m_currentByte;
    this->GetStream().get((char&)this->m_currentByte);
    this->m_currentBitNum = 1;
}

std::string InputBitStream::GetNextBit()
{
    this->m_currentBitNum += 1;
    if (this->m_currentBitNum > CHAR_BIT)
    {
        this->Consume();
    }

    int bit = this->m_currentByte >> (CHAR_BIT - this->m_currentBitNum);
    bit = bit & 1; //apply mask to get rid of everything but first bit


    if (bit)
    {
        return "1";
    }
    else
    {
        return "0";
    }
}

bool InputBitStream::isGood()const
{
    return !this->isEOF();
}

bool InputBitStream::isEOF() const
{
    //First peek, because if we are at last byte not counting EOF we want flag to be raised
    this->GetStream().peek();
    bool endOfFile = this->GetStream().eof();
    bool pastPadding = this->m_paddingBitLength >= (CHAR_BIT - this->m_currentBitNum);
    //if we are EOF and past padding, then we are truly EOF
    if (endOfFile && !pastPadding)
    {
        this->GetStream().clear(); //clear the EOF bit since we have few more bits to process
    }
    return this->GetStream().eof();
}
