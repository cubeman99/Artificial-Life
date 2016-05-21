#include "CircleGenome.h"
#include <string.h>
#include "util/Random.h"


namespace circle_fitting
{

Genome::Genome()
	: m_mutationRate(0.0f)
{
	m_numBytes = 0;
}

Genome::Genome(int numBytes)
	: m_mutationRate(0.0f)
{
	m_bytes.resize(numBytes);
	m_numBytes = (int) m_bytes.size();
}

int Genome::GetNumBytes() const
{
	return m_numBytes;
}

unsigned char Genome::GetByte(int index) const
{
	return m_bytes[index];
}

void Genome::SetByte(int index, unsigned char value)
{
	m_bytes[index] = value;
}

bool Genome::GetBit(int bitIndex) const
{
	int byteIndex = bitIndex / 8;
	int bit       = bitIndex % 8;

	unsigned char bitMask = (unsigned char) (1 << (7 - bit));
	return (m_bytes[byteIndex] & bitMask) != 0;
}

void Genome::SetBit(int bitIndex, bool value)
{
	int byteIndex = bitIndex / 8;
	int bit       = bitIndex % 8;

	unsigned char bitMask = (unsigned char) (1 << (7 - bit));
	if (value)
		m_bytes[byteIndex] |= bitMask;
	else
		m_bytes[byteIndex] &= ~bitMask;
}

void Genome::Crossover(Genome* parent1, Genome* parent2, bool mutate)
{
	int numCrossoverPoints = 1;
	int* crossoverPoints = new int[numCrossoverPoints];

	// Create random crossover points.
	for (int i = 0; i < numCrossoverPoints; i++)
	{
		int point = Random::NextInt(0, m_numBytes * 8);
			
		//bool equal;
		/*do
		{
			point = Random::NextInt(0, m_numBytes);
			equal = false;

			for (int j = 0; j < i; j++)
			{
				if (point == crossoverPoints[j])
				{
					equal = true;
					break;
				}
			}
		}
		while (equal);*/

		// Insert crossover point in sorted order.
		if (i == 0)
			crossoverPoints[0] = point;
		else if (i > 0 && point >= crossoverPoints[i - 1])
			crossoverPoints[i] = point;
		else
		{
			for (int j = 0; j < i; j++)
			{
				if (point < crossoverPoints[j])
				{
					for (int k = i; k > j; k--)
						crossoverPoints[k] = crossoverPoints[k - 1];
					crossoverPoints[j] = point;
					break;
				}
				else if (j == i - 1)
					crossoverPoints[j] = point;
			}
		}
	}

	// Crossover the bits of the two parent genomes.
	int parentIndex = (Random::NextBool() ? 1 : 0);
	Genome* parents[2] = { parent1, parent2 };
	Genome* parent = parents[parentIndex];
	
	for (int i = 0; i < numCrossoverPoints + 1; i++)
	{
		// Copy the sequence of bytes.
		int begIndex = 0;
		int endIndex = m_numBytes * 8;
		if (i > 0)
			begIndex = crossoverPoints[i - 1];
		if (i < numCrossoverPoints)
			endIndex = crossoverPoints[i];

		for (int j = begIndex; j < endIndex; j++)
			SetBit(j, parent->GetBit(j));

		//memcpy_s(&m_bytes[0] + begIndex, m_numBytes,
				 //&parent->m_bytes[0] + begIndex,
				 //endIndex - begIndex);

		// Switch parents for the next section.
		parentIndex = 1 - parentIndex;
		parent = parents[parentIndex];
	}


	delete [] crossoverPoints;

	if (mutate)
		Mutate();
}

void Genome::Mutate()
{
	// Randomize the bits of the genome.
	for (int i = 0; i < m_numBytes; i++)
	{
		for (int bit = 0; bit < 8; bit++)
		{
			if (Random::NextFloat() < m_mutationRate)
				m_bytes[i] ^= char(1 << (7 - bit)); // xor with 1 will flip the bit.
		}
	}
}

void Genome::Randomize()
{
	// Randomize the bits of the genome.
	for (int i = 0; i < m_numBytes; i++)
	{
		m_bytes[i] = 0;

		for (int bit = 0; bit < 8; bit++)
		{
			if (Random::NextBool())
				m_bytes[i] |= char(1 << (7 - bit));
		}
	}
}

std::string Genome::GetBitString() const
{
	std::string str;

	for (int i = 0; i < m_numBytes; i++)
	{
		for (int bit = 0; bit < 8; bit++)
		{
			// Bits from left to right corresponding to little endian machines.
			if ((m_bytes[i] & char(1 << (7 - bit))) != 0)
				str.push_back('1');
			else
				str.push_back('0');
		}
		str.push_back(' ');
	}

	return str;
}

};
