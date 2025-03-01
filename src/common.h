#ifndef HELPER_H
#define HELPER_H

#include <cstdint>
#include <iostream>
#include <map>
#include <vector>
#include <sstream>
#include <algorithm>

class Mp4;

typedef unsigned int uint;
typedef unsigned char uchar;

enum LogMode { E, W, I, W2, V, VV };
extern LogMode g_log_mode;
extern size_t g_max_partsize;
extern bool g_interactive, g_muted, g_ignore_unknown, g_stretch_video,
    g_show_tracks, g_dont_write, g_use_chunk_stats, g_dont_exclude, g_dump_repaired;
extern const bool is_new_ffmpeg_api;
extern std::string g_version_str;
extern uint g_num_w2;  // hidden warnings
extern Mp4* g_mp4;
extern void (*g_onProgress)(int);
extern void (*g_onStatus)(const std::string&);

#define UNFOLD_PARAM_PACK(pack, os) \
	_Pragma("GCC diagnostic push"); \
	_Pragma("GCC diagnostic ignored \"-Wunused-value\""); \
	using creator = int[]; \
	creator{ 0, ( os << (std::forward<decltype(pack)>(pack)), 0) ... }; \
	_Pragma("GCC diagnostic pop"); \


template<class... Args>
void logg(Args&&... args){
//	(std::cout << ... << args); // Binary left fold (c++17)
	UNFOLD_PARAM_PACK(args, std::cout)
}

template<class... Args>
void logg(LogMode m, Args&&... x){
	if (g_log_mode < m) {
		if (m == W2) g_num_w2++;
		return;
	}
	if (m == I)
		std::cout << "Info: ";
	else if (m == W || m == W2)
		std::cout << "Warning: ";
	else if (m == E)
		std::cout << "Error: ";
	logg(std::forward<Args>(x)...);
}

template<class... Args>
std::string ss(Args&&... args){
	std::stringstream ss;
	UNFOLD_PARAM_PACK(args, ss)
	return ss.str();
}

bool contains(const std::initializer_list<std::string>& c, const std::string& v);

template <typename Container>
bool contains(Container const& c, typename Container::const_reference v) {
  return std::find(c.begin(), c.end(), v) != c.end();
}

/* NAL unit types */
enum {
	NAL_SLICE           = 1, //non keyframe
	NAL_DPA             = 2,
	NAL_DPB             = 3,
	NAL_DPC             = 4,
	NAL_IDR_SLICE       = 5, //keyframe
	NAL_SEI             = 6,
	NAL_SPS             = 7,
	NAL_PPS             = 8,
	NAL_AUD             = 9,
	NAL_END_SEQUENCE    = 10,
	NAL_END_STREAM      = 11,
	NAL_FILLER_DATA     = 12,
	NAL_SPS_EXT         = 13,
	NAL_AUXILIARY_SLICE = 19,
	NAL_FF_IGNORE       = 0xff0f001,
};


const std::map<std::string, std::string> g_atom_names = {
    {"esds", "ES Descriptor"},
    {"stsd", "sample description"},
    {"minf", "media information"},
    {"stss", "sync samples"},
    {"udta", "user data"},
    {"stsz", "sample to size"},
    {"ctts", "sample to composition time"},
    {"stsc", "sample to chunk"},
    {"stts", "sample to decode time"},
    {"co64", "chunk to offset 64"},
    {"stco", "chunk to offset"},
    {"mvhd", "movie header"},
    {"mdhd", "media header"}
};

void mute();
void unmute();

uint16_t swap16(uint16_t us);
uint32_t swap32(uint32_t ui);
uint64_t swap64(uint64_t ull);

void outProgress(double now, double all);

int readGolomb(const uchar *&buffer, int &offset);
uint readBits(int n, const uchar *&buffer, int &offset);

void printBuffer(const uchar* pos, int n);
std::string mkHexStr(const uchar* pos, int n, int seperate_each=0);

void hitEnterToContinue(bool new_line=true);

std::string pretty_bytes(double bytes);
void printVersion();

void chkHiddenWarnings();

void trim_right(std::string& in);

std::string getMovExtension(const std::string& path);
std::string getOutputSuffix();

double calcEntropy(const std::vector<uchar>& in);

#define to_uint(a) static_cast<unsigned int>(a)
#define to_size_t(a) static_cast<size_t>(a)

class Atom;
// this class is meant for reading/writing mvhd and mdhd
class HasHeaderAtom {
public:
	int timescale_;
	int64_t duration_;

	static void editHeaderAtom(Atom* header_atom, int64_t duration, bool is_tkhd=false);
	void editHeaderAtom();
	void readHeaderAtom();
	int getDurationInMs();

protected:
	Atom* header_atom_;
};

#endif // HELPER_H
