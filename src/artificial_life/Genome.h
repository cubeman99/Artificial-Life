#ifndef _GENOME_H_
#define _GENOME_H_

#include <vector>


class Gene
{
public:
	Gene(int offset, unsigned char* value)
		: m_offset(offset)
		, m_value(value)
	{}

	unsigned char AsByte() const { return *m_value; }

	float AsFloat() const { return ((float) *m_value / 255.0f); }

	int AsInt(int rangeMin, int rangeMax) const
	{
		return (rangeMin + (int) (((float) *m_value / 256.0f) * (float) (rangeMax - rangeMin + 1)));
	}
	
	float AsFloat(float rangeMin, float rangeMax) const
	{
		return (rangeMin + (((float) *m_value / 255.0f) * (rangeMax - rangeMin)));
	}

private:
	int m_offset;
	unsigned char* m_value;
};


class Genome
{
public:
	Genome();
	virtual ~Genome();

	void InitSize(int size);

	void CopyFrom(Genome* genome);

	Gene GetGene(int offset) { return Gene(offset, &m_data[offset]); }

	unsigned char	GetGeneValue(int index) const;
	int				GetGeneValue(int index, int rangeMin, int rangeMax) const;
	float			GetGeneValue(int index, float rangeMin, float rangeMax) const;

	unsigned char* GetData() { return &m_data[0]; }
	int GetDataSize() { return (int) m_data.size(); }
	
	virtual void Mutate() {}

	void Randomize();
	void Mutate(float mutationRate);
	void Crossover(Genome* g1, Genome* g2);

private:
	std::vector<unsigned char> m_data;
};


#endif // _GENOME_H_