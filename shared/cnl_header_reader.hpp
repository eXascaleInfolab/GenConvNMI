//! \brief CNL format header reader
//! ATTENTION: this file is shared for both gecmi and onmi
//!
//! \license Apache License, Version 2.0: http://www.apache.org/licenses/LICENSE-2.0.html
//! > 	Simple explanation: https://tldrlegal.com/license/apache-license-2.0-(apache-2.0)
//!
//! Copyright (c)
//! \authr Artem Lutov
//! \email luart@ya.ru
//! \date 2017-02-10

#ifndef CNL_HEADER_READER
#define CNL_HEADER_READER

#include <sstream>  // istringstream
#include <string>
#include <cstring>  // strtok
#include <cmath>  // sqrt

#ifdef __unix__
#include <sys/stat.h>
#endif // __unix__


using std::string;
using std::istream;
using std::istringstream;

//! \brief  Parse the header of CNL file and validate the results
//! \post clsnum <= ndsnum if ndsnum > 0. 0 means not specified
//!
//! \param fsm istream&  - the reading file
//! \param line string&  - processing line (string, header) being read from the file
//! \param[out] clsnum size_t&  - resulting number of clusters if specified, 0 in case of parsing errors
//! \param[out] ndsnum size_t&  - resulting number of nodes if specified, 0 in case of parsing errors
//! \return void
void parseHeader(istream& fsm, string& line, size_t& clsnum, size_t& ndsnum)
{
	// Process the header, which is a special initial comment
	// The target header is:  # Clusters: <cls_num>[,] Nodes: <cls_num>
	const string  clsmark = "clusters";
	const string  ndsmark = "nodes";
	while(getline(fsm, line)) {
		// Skip empty lines
		if(line.empty())
			continue;
		// Consider only subsequent comments
		if(line[0] != '#')
			break;

		// 1. Replace the staring comment mark '#' with space to allow "#clusters:"
		// 2. Replace ':' with space to allow "Clusters:<clsnum>"
		for(size_t pos = 0; pos != string::npos; pos = line.find(':', pos + 1))
			line[pos] = ' ';

		istringstream fields(line);
		string field;

		// Read clusters specification
		fields >> field;
		if(field.length() != clsmark.length())
			continue;
		// Convert to lower case
		for(size_t i = 0; i < field.length(); ++i)
			field[i] = tolower(field[i]);
		if(field != clsmark)
			continue;
		fields >> clsnum;

		// Read nodes specification
		fields >> field;
		// Allow optional ','
		if(!field.empty() && field[0] == ',')
			fields >> field;
		if(field.length() == ndsmark.length()) {
			for(size_t i = 0; i < field.length(); ++i)
				field[i] = tolower(field[i]);
			if(field == ndsmark)
				fields >> ndsnum;
		}
		// Validate and correct the number of clusters if required
		// Note: it's better to reallocate a container a few times than too much overconsume the memory
		if(ndsnum && clsnum > ndsnum) {
			fprintf(stderr, "WARNING parseHeader(), clsnum (%lu) should not increase ndsnum (%lu)"
				", fixed\n", clsnum, ndsnum);
			clsnum = ndsnum;
			//assert(0 && "parseHeader(), clsnum typically should be less than ndsnum");
		}
		// Get following line for the unified subsequent processing
		getline(fsm, line);
		break;
	}
}

//! \brief Estimate zeroized values
//!
//! \param ndsnum size_t&  - the estimate number of nodes if 0, otherwise omit it
//! \param clsnum size_t&  - the estimate number of clusters if 0, otherwise omit it
//! \param cmsbytes size_t  - the number of bytes in the file
//! \param membership=1.f float  - average membership of nodes in the clusters, > 0,
//! 	=> 1 in case all nodes belong to any cluster and overlaps are present (> 1)
//! \return bool  - the estimation is made
bool estimateSizes(size_t& ndsnum, size_t& clsnum, size_t cmsbytes, float membership=1.f) noexcept
{
	if(membership <= 0) {
		fprintf(stderr, "WARNING estimateSizes(), invalid membership = %G specified"
			", reseted to 1\n", membership);
		membership = 1;
//		throw invalid_argument("estimateSizes(), membership = " + to_string(membership)
//			+ " should be positive");
	}

	if(clsnum && !ndsnum) {
		// Typically the number of nodes is at least square of the number of clusters
		// Note: optimistic estimate to not overuse the memory
		ndsnum = 2 * clsnum;  // clsnum * clsnum / membership;
		return true;
	}

	bool  estimated = false;
	if(!ndsnum) {
		// Estimate the number of nodes from the file size
		if(!cmsbytes)
			return false;

		size_t  magn = 10;  // Decimal ids magnitude
		unsigned  img = 1;  // Index of the magnitude (10^1)
		size_t  reminder = cmsbytes % magn;  // Reminder in bytes
		ndsnum = reminder / ++img;  //  img digits + 1 delimiter for each element
		while(cmsbytes >= magn) {
			magn *= 10;
			ndsnum += (cmsbytes - reminder) % magn / ++img;
			reminder = cmsbytes % magn;
		}
		ndsnum /= membership;
		estimated = true;
	}

	// Usually the number of clusters does not increase square root of the number of nodes
	if(!clsnum) {
		clsnum = sqrt(ndsnum * membership) + 1;  // Note: +1 to consider rounding down
		estimated = true;
	}

	return estimated;
}

//! \brief Identify size of the input in bytes
//!
//! \param input istream&  - input stream
//! \param fname=nullptr const char*  - file name of the corresponding input stream
//! \return size_t  - resulting size of the input or 0 on errors
size_t inputSize(istream& input, const char* fname=nullptr)
{
	size_t  inpsize = 0;
#ifdef __unix__
	if(fname) {
		struct stat  filest;
		if(!stat(fname, &filest))
			inpsize = filest.st_size;
	}
	//fprintf(stderr, "# %s: %lu bytes\n", fname, inpsize);
#endif // __unix
	// Get length of the file
	if(!inpsize) {
		input.seekg(0, input.end);
		inpsize = input.tellg();  // The number of bytes in the input communities
		if(inpsize == size_t(-1)) {
			//assert(!input && "inpsize = -1 should be only for the failed file operation");
			fputs("WARNING inputSize(), file size evaluation failed\n", stderr);
			inpsize = 0;
		}
		input.seekg(0, input.beg);
	}

	return inpsize;
}

#endif // CNL_HEADER_READER
