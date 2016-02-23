#include <arpa/inet.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>

#include <ctime>

#include <ctype.h>

#include <map>

#include <random>

#include <utility>
#include <vector>


struct token
{
    std::string text;
    bool is_initial = false;

    bool operator <(const token &rhs) const
    {
	return std::tie(text, is_initial) < std::tie(rhs.text, rhs.is_initial);
    }
};

class MarkovModel{
    std::map< token, float > initials;
    std::map< token, std::map< token, float > > transitions;

public:
    MarkovModel (std::vector< token > tokens);
    token sample_model_initial(double p);
    token sample_model_transition(token t, double p);
    std::vector< token > sample_sentence();
};


std::vector< token > load_tokens(std::string path)
{
    std::ifstream is(path);

    std::vector<token> words;

    std::string word;
    char c;
    bool is_initial = true;
    while(is.get(c))
    {
	if(c == ' ' || c == '\n')// end of a wordn
	{
	    if(word.length() > 0) // don't add blank words
	    {
		token t;
		t.text = word;
		t.is_initial = is_initial;

		words.push_back(t);
	    }

	    word = "";
	    is_initial = false;
	}
	else if (c == '.')
	{
	    is_initial = true;
	}
	else if(isalpha(c))
	{
	    word += c;
	}
    }

    return words;
}

MarkovModel::MarkovModel(std::vector<token> tokens)
{
    int num_initials = 0;

    std::map<token, uint32_t> transition_counts;

    // count the occurrences of everything
    for(uint32_t i=0;i<tokens.size() - 1;i++)
    {
	token t = tokens[i];
	if(t.is_initial)
	{
	    initials[t] += 1;
	    num_initials += 1;
	}

	token tt = tokens[i+1];
	if(!tt.is_initial)
	{
	    transitions[t][tt] += 1;
	    transition_counts[t] += 1;
	}
    }

    // turn these values into probabilities

    for(auto& x: initials)
    {
	x.second /= num_initials;
    }

    for(auto& x: transitions)
    {
	for(auto& y: x.second)
	{
	    y.second /= transition_counts[x.first];
	}
    }
}


token MarkovModel::sample_model_initial(double p)
{
    token t;
    if( p > 0 && p < 1 )
    {
	double x = 0;

	for(auto it = initials.begin(); it != initials.end(); ++it)
	{
	    x += it->second;
	    if(x > p)
	    {
		t = it->first;
		break;
	    }
	}
    }
    return t;
}

token MarkovModel::sample_model_transition(token t, double p)
{
    token tt;
    if( p > 0 && p < 1 )
    {
	double x = 0;

	std::map<token, float> successors = transitions[t];
	for(auto it=successors.begin();it != successors.end(); ++it)
	{
	    x += it->second;

	    if(x > p)
	    {
		tt = it->first;
		break;
	    }
	}
    }
    return tt;
}

std::vector<token> MarkovModel::sample_sentence()
{
    unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count();

    std::default_random_engine generator (seed1);
    std::uniform_real_distribution<double> distribution(0.0,1.0);


    std::vector<token> sentence;

    token t = sample_model_initial(distribution(generator));
    sentence.push_back(t);

    // sample an initial character
    for(uint32_t i=0; i < 20; i++)
    {
	t = sample_model_transition(t, distribution(generator));
	sentence.push_back(t);
    }

    return sentence;
}

int main (int argc, char* argv[])
{
    if(argc == 2)
    {
	std::string path = argv[1];
	std::cout << "loading from: '" << path << "'" << std::endl;
	std::vector<token> tokens = load_tokens(path);

	MarkovModel m = MarkovModel(tokens);

	unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count();

	std::default_random_engine generator (seed1);
	std::uniform_real_distribution<double> distribution(0.0,1.0);
	std::clock_t    start;

	start = std::clock();
	// your test
	m.sample_model_initial(0.99);
	std::cout << "Time: " << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000) << " ms" << std::endl;
    }
}
