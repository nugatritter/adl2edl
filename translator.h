#include <cstdio>
#include <string>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>


#include <iostream> 
#include <fstream> 
#ifdef HPUX
#include <ostream>
#endif
#include <list>
#include <vector>
#include <algorithm>


//shell command in shell_cmd.h
#define SHCMDC_MAJOR_VERSION 4
#define SHCMDC_MINOR_VERSION 0
#define SHCMDC_RELEASE 0

//message button in message_button.h
#define MSGBTC_MAJOR_VERSION 4
#define MSGBTC_MINOR_VERSION 0
#define MSGBTC_RELEASE 0

//choice button in button.h
#define BTC_MAJOR_VERSION 4
#define BTC_MINOR_VERSION 0
#define BTC_RELEASE 0

// byte monitor in pvFactory/byte.h
#define BYTE_MAJOR 4
#define BYTE_MINOR 0
#define BYTE_RELEASE 0

//related_display.h
#define RDC_MAJOR_VERSION 4
#define RDC_MINOR_VERSION 0
#define RDC_RELEASE 0

//xygraph.h xyplot
#define XYGC_MAJOR_VERSION 4
#define XYGC_MINOR_VERSION 0
#define XYGC_RELEASE 0

//bar.h bar
#define BARC_MAJOR_VERSION 4
#define BARC_MINOR_VERSION 0
#define BARC_RELEASE 0

//meter.h meter
#define METERC_MAJOR_VERSION 4
#define METERC_MINOR_VERSION 0
#define METERC_RELEASE 0

// text entry(adl) text control(edl)
// text update(adl) text monitor(edl)
//x_text_dsp_obj.h text monitor
#define XTDC_MAJOR_VERSION 4
#define XTDC_MINOR_VERSION 7
#define XTDC_RELEASE 0

//x_text_obj.h text
// for static text(edl)
#define AXTC_MAJOR_VERSION 4
#define AXTC_MINOR_VERSION 1
#define AXTC_RELEASE 1

//line_obj.h polyline
#define ALC_MAJOR_VERSION 4
#define ALC_MINOR_VERSION 0
#define ALC_RELEASE 0

//circle_obj.h circle
#define ACC_MAJOR_VERSION 2
#define ACC_MINOR_VERSION 1
#define ACC_RELEASE 0

//arc_obj.h arc
#define AAC_MAJOR_VERSION 2
#define AAC_MINOR_VERSION 1
#define AAC_RELEASE 0

// rectangle_obj.h 
#define ARC_MAJOR_VERSION 4
#define ARC_MINOR_VERSION 0
#define ARC_RELEASE 0

// menu_button.h 
#define MBTC_MAJOR_VERSION 4
#define MBTC_MINOR_VERSION 0
#define MBTC_RELEASE 0

//act_win.h header
#define AWC_MAJOR_VERSION 4
#define AWC_MINOR_VERSION 0
#define AWC_RELEASE 1

//group.h text
#define AGC_MAJOR_VERSION 4
#define AGC_MINOR_VERSION 0
#define AGC_RELEASE 0

//pip.h text
#define PIP_MAJOR_VERSION 4
#define PIP_MINOR_VERSION 1
#define PIP_RELEASE 0

// dsym_obj.h 
#define ARC_MAJOR_VERSION 4
#define ARC_MINOR_VERSION 0
#define ARC_RELEASE 0

struct battr {
		int clr;
		int style;
		int fill;
		int width;
};

struct dattr {
		int colormode;	// static, alarm, or discrete
		int vis;
		std::string calc;
		std::string chan;
		std::string chanB;
		std::string chanC;
		std::string chanD;
};

class parseclass
{
public:
	parseclass();
	virtual ~parseclass();
	void stripQs(std::string &str);
	std::string line;
    int pos = 0;
    int eq_pos = 0;
    int open = 0;
    std::string bopen;
    std::string bclose;
    std::string eq;
    std::string space;
    std::string nil;
	std::string squote;
    int snum = 0;
    char s1[256], s2[256];
};

class comclass
{
public:
	comclass();
	virtual ~comclass();
	void fill_attrs(int attr);
	int x = 0;
    int y = 0;
    int wid = 0;
    int hgt = 0;
	int clr = 0;
	int bclr = -1;

	int style = 0;
	int fill = 0;
	int linewidth = 0;
	int colormode = 0;	// static, alarm, or discrete

	int vis = 0;		// static, if not zero, if zero, calc
	int visInverted = 0;

	std::string calc;
	std::string chan;
	std::string chanB;
	std::string chanC;
	std::string chanD;
	std::string graphicRule;
};

class shellclass : public comclass, public parseclass
{
public:
	shellclass(int attr);
	virtual ~shellclass();
	int parse(std::ifstream&, std::ostream&, std::ostream&, bool isMenuItem = false);
	int bclr;
};

class valclass : public comclass, parseclass
{
public:
	valclass(int attr);
	virtual ~valclass();
	int parse(std::ifstream&, std::ostream&, std::ostream&);
	int bclr = 0;
};

class mbuttonclass : public comclass, parseclass
{
public:
	mbuttonclass(int attr);
	virtual ~mbuttonclass();
	int parse(std::ifstream&, std::ostream&, std::ostream&);
	int bclr;
	std::string label;
	std::string press_msg;
	std::string release_msg;
};

class buttonclass : public comclass, parseclass
{
public:
	buttonclass(int attr);
	virtual ~buttonclass();
	int parse(std::ifstream&, std::ostream&, std::ostream&);
	int bclr;
};

class byteclass : public comclass, public parseclass
{
public:
	byteclass(int attr);
	virtual ~byteclass();
	int parse(std::ifstream&, std::ostream&, std::ostream&);
	int bclr = 0;
	int direction = 0;
	int sbit = 0;
	int ebit = 0;
};

class shellnode 
{
public:
    shellnode();
    virtual ~shellnode();
    std::string name;
    std::string label;
    std::string args;
    std::string type;
    std::string command;
};

class relnode 
{
public:
    relnode();
    virtual ~relnode();
    std::string name;
    std::string label;
    std::string args;
	std::string policy;
};

class tnode{
public:
    tnode();
    virtual ~tnode();
    std::string xdata;
    std::string ydata;
    int color = 0;
};


class relatedclass : public comclass, public parseclass
{
public:
	relatedclass(int attr);
	virtual ~relatedclass();
	int parse(std::ifstream&, std::ostream&, std::ostream&);
	std::string look_for_file(std::ostream&, std::string tname);
	int bclr = 0;
};

class xyclass :public comclass, public parseclass
{
public:
	xyclass(int attr);
	virtual ~xyclass();
	int parse(std::ifstream&, std::ostream&, std::ostream&);
	//int clr;
	int bgColor = 0;
	int plotMode = 0;
	int border = 0;
	int count = 0;
	int updateTimerValue = 0;
	int xAxis = 0;
	int xAxisStyle = 0;
	int xAxisSource = 0;
	int xMin1 = 0;
	int xMin2 = 0;
	int xMax1 = 0;
	int xMax2 = 0;
	int xAxisTimeFormat = 0;
	int symbol = 0;

	std::string title;
	std::string xlabel;
	std::string ylabel;
	std::string squote;
	int erase = 0;

	// Axes:
	// if range is "from pv", that value is not in adl file
	int xrangeStyle = 0;
	int y1rangeStyle = 0;
	int y2rangeStyle = 0;
	// if min = 0 or max = 1, that value is not in adl file
	double xmin = 0;
	double xmax = 0;
	double y1min = 0;
	double y1max = 0;
	double y2min = 0;
	double y2max = 0;
};

class stripclass :public comclass, public parseclass
{
public:
	stripclass(int attr);
	virtual ~stripclass();
	int parse(std::ifstream&, std::ostream&, std::ostream&);
	//int clr;
	int bgColor = 0;
	int plotMode = 0;
	int border = 0;
	int count = 0;
	int updateTimerValue = 0;
	int xAxis = 0;
	int xAxisStyle = 0;
	int xAxisSource = 0;
	int xMin1 = 0;
	int xMin2 = 0;
	int xMax1 = 0;
	int xMax2 = 0;
	int xAxisTimeFormat = 0;
	int symbol = 0;

	std::string title;
	std::string xlabel;
	std::string ylabel;
	std::string squote;
	int erase = 0;

	// Axes:
	// if range is "from pv", that value is not in adl file
	int xrangeStyle = 0;
	int y1rangeStyle = 0;
	int y2rangeStyle = 0;
	// if min = 0 or max = 1, that value is not in adl file
	double xmin = 0;
	double xmax = 0;
	double y1min = 0;
	double y1max = 0;
	double y2min = 0;
	double y2max = 0;
};

class barclass : public comclass, public parseclass
{
public:
	barclass(int attr);
	virtual ~barclass();
	int parse(std::ifstream&, std::ostream&, std::ostream&);
	int bgColor = 0;
	std::string label;
	bool showScale = true;
	bool showAlarmLimits = true;
	std::string scaleType = "";
};

class meterclass :public comclass, public parseclass
{
public:
	meterclass(int attr);
	virtual ~meterclass();
	int parse(std::ifstream&, std::ostream&, std::ostream&);

	int meterColorMode = 0;
	int scaleColor = 0;
	int scaleColorMode = 0;
	int labelColor = 0;
	int fgColor = 0;
	int bgColor = 0;
	int tsColor = 0;
	int bsColor = 0;

	std::string controlPvExpStr;
	std::string readPvExpStr;
	std::string literalLabel;
	std::string label;
	int labelType = 0;
	int showScale = 0;
	std::string scaleFormat;
	int scalePrecision = 0;
	int scaleLimitsFromDb = 0;
	int useDisplayBg = 0;
	int majorIntervals = 0;
	int minorIntervals = 0;
	int needleType = 0;
	int shadowMode = 0;
	int scaleMin = 0;
	int scaleMax = 0;
	std::string labelFontTag;
	std::string scaleFontTag;
	int meterAngle = 0;
};

class menuclass :public comclass, public parseclass
{
public:
    menuclass(int attr);
    virtual ~menuclass();
	int parse(std::ifstream&, std::ostream&, std::ostream&);
	int bclr = 0;
};

class textmonclass :public comclass, public parseclass
{
public:
    textmonclass(int attr);
    virtual ~textmonclass();
	int parse(std::ifstream&, std::ostream&, std::ostream&, int noedit);
	int bclr = 0;
	std::string format;
};

class polyclass : public comclass, public parseclass
{
public:
	polyclass(int attr);
	virtual ~polyclass();
	int parse(std::ifstream&, std::ostream&, std::ostream&, int close);
	int fillColor = 0;
	int close = 0;
	int arrows = 0;
};

class circleclass : public comclass, public parseclass
{
public:
	circleclass(int attr);
	virtual ~circleclass();
	int parse(std::ifstream&, std::ostream&, std::ostream&);
	int width = 0;
	int fillColor = 0;
};

class arcclass : public comclass, public parseclass
{
public:
	arcclass(int attr);
	virtual ~arcclass();
	int parse(std::ifstream&, std::ostream&, std::ostream&);
	int fillColorMode = 0;
	int startAng = 0;
	int startAng2 = 0;
	int totalAng = 0;
	int totalAng2 = 0;
	int fillMode = 0;
};


class cmapclass : public comclass, public parseclass
{
public:
	cmapclass();
	virtual ~cmapclass();
	int parse(std::ifstream&, std::ostream&, std::ostream&);
	std::string& getRGB(int index) ;
private:
	std::vector <std::string> rgbvec;
	int numColors = 0;
};

class rectclass : public comclass, public parseclass
{
public:
	rectclass(int attr);
	virtual ~rectclass();
	int parse(std::ifstream&, std::ostream&, std::ostream&);
};

class textclass : public comclass, public parseclass
{
public:
	textclass(int attrs);
	virtual ~textclass();
	int parse(std::ifstream&, std::ostream&, std::ostream&);

	int bkClr = 0;
	std::string textstr;
};
	
class headclass :public comclass, public parseclass
{
public:
	headclass();
	virtual ~headclass();
	int parse(std::ifstream&, std::ostream&, std::ostream&);

	int fgClr = 0;
	int bkClr = 0;
	int gridShow = 0; 		// gridShow 0, Active 0, Spacing 10, orthogonal 0
	int gridSpacing = 5; 	//5
	int gridOn = 0; 		//0
	int snapToGrid = 0; 	//0
};
class dsymclass : public comclass, public parseclass
{
public:
	dsymclass(int attr);
	virtual ~dsymclass();
	int parse(std::ifstream&, std::ostream&, std::ostream&);
};
class gifclass : public comclass, public parseclass
{
public:
	gifclass(int attr);
	virtual ~gifclass();
	int parse(std::ifstream&, std::ostream&, std::ostream&);
};
class translator
{
public:
	translator();
	~translator();
	int processFile (char *in, char *out);
	int parseComposite(std::ifstream&, std::ostream&, 
		std::ostream&, std::ostream&);
	int parseCompositeClose(std::ifstream&, std::ostream&, 
		std::ostream&, std::ostream&);
	int read_battr(std::ifstream&, std::ostream&);
	int read_dattr(std::ifstream&, std::ostream&);
	std::ifstream inf;
	std::ofstream toutf;
	static std::string dir;
	static int line_ctr;
	static int display_height;
	static int display_width;
	static bool debug;
	static struct battr ba;
	static struct dattr da;
private:
	std::string medmfile;
	std::string edmfile;
};

static cmapclass cmap;
