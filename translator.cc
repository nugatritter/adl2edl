#include <stdlib.h>
#include <unistd.h>
#include <map>

#include <iostream>
#include <sstream>
#include "translator.h"
#include "fonts.h"

using std::endl;
using std::cerr;
using std::cout;
using std::vector;
using std::ifstream;
using std::ofstream;
using std::ostream;
using std::string;
using std::map;
using std::ostringstream;

static string version = "1.9.1 (" + string(__DATE__) + ", " + string(__TIME__)
		+ ")";

map<string, string> graphicRuleMap;
string translator::dir;
int translator::line_ctr;
int translator::display_height;
int translator::display_width;
struct battr translator::ba;
struct dattr translator::da;

bool urgb;
bool retitle; // already used 'rename'
int default_clr = 14;
int default_bclr = 3;
int debugAdl2Edl = 0;

fontInfoClass fi;
static char *fptr;

void usage() {
	cout << "Version " << version << endl;
	cout << "usage: [-h] [-rgb] [-rename] [-d LEVEL] medm_file [edm_file]"
			<< endl;
	cout << "   medm_file is the name of an medm file to translate to edm."
			<< endl;
	cout << "      medm_file will typically end in .adl" << endl;
	cout << "   edm_file is the edm filename." << endl;
	cout << "      If not used, the translation is written to stdout" << endl;
	cout << "      and diagnostic output to ${medm_file}.adl.db" << endl;
	cout << "Options:" << endl;
	cout << " -h          show this help message and exit" << endl;
	cout << " -rgb        use rgb mode to specify colors in the edm translation"
			<< endl;
	cout << " -rename     rename the medm file substituting .edl for .adl"
			<< endl;
	cout << " -f fontFile set the font filename" << endl;
	cout << " -d LEVEL    set the diagnostic output to LEVEL" << endl;
}

class Dm2kGraphicRuleParser {
	const string whitespace = " \t";
	const string fill = " ";
	const string numbers = "-.0123456789";
public:
	string trim(const string &str) {
		const auto strBegin = str.find_first_not_of(whitespace);
		if (strBegin == string::npos) {
			return "";
		}
		const auto strEnd = str.find_last_not_of(whitespace);
		const auto strRange = strEnd - strBegin + 1;
		return str.substr(strBegin, strRange);
	}

	string reduce(const string &str) {
		auto result = trim(str);
		if (result[0] == ';')
			return "";
		auto beginSpace = result.find_first_of(whitespace);
		while (beginSpace != string::npos) {
			const auto endSpace = result.find_first_not_of(whitespace,
					beginSpace);
			const auto range = endSpace - beginSpace;
			result = result.replace(beginSpace, range, fill);
			const auto newStart = beginSpace + fill.length();
			beginSpace = result.find_first_of(whitespace, newStart);
		}
		return result;
	}

	bool startsWithNumber(const string &str) {
		auto begin = str.find_first_of(numbers);
		if (begin != string::npos && begin == 0) {
			return true;
		}
		return false;
	}

	void getTwoNumbers(const string &str, string &firstNumberStr,
			string &secondNumberStr) {
		auto strBegin = str.find_first_not_of(whitespace);
		if (strBegin == string::npos) {
			firstNumberStr = "";
			secondNumberStr = "";
			return;
		}
		auto strEnd = str.find_first_of(whitespace, strBegin);
		if (strEnd == string::npos) {
			firstNumberStr = "";
			secondNumberStr = "";
			return;
		}
		auto strRange = strEnd - strBegin;
		firstNumberStr = str.substr(strBegin, strRange);
		strBegin = str.find_first_not_of(whitespace, strEnd);
		if (strBegin == string::npos) {
			secondNumberStr = "";
			return;
		}
		strEnd = str.find_first_of(whitespace, strBegin);
		if (strEnd == string::npos) {
			secondNumberStr = "";
			return;
		}
		strRange = strEnd - strBegin;
		secondNumberStr = str.substr(strBegin, strRange);
	}

	void getGraphicRuleName(const string &str, string &name) {
		auto strBegin = str.find_first_of("graphicRule");
		if (strBegin == string::npos) {
			name = "";
			return;
		}
		strBegin = str.find_first_of(whitespace, strBegin);
		if (strBegin == string::npos) {
			name = "";
			return;
		}
		strBegin = str.find_first_not_of(whitespace, strBegin);
		if (strBegin == string::npos) {
			name = "";
			return;
		}
		name = str.substr(strBegin);
	}

	int parse(char *dm2kGraphicRuleFile, char *prefix) {
		string line;
		string graphicRuleName;
		string firstNumber[32], secondNumber[32];
		ostringstream os;
		string result = "";
		int numStates = 0;
		if (!dm2kGraphicRuleFile) {
			cerr << "missing dm2kGraphicRuleFile" << endl;
			return 0;
		}
		ifstream inf(dm2kGraphicRuleFile);
		if (!inf) {
			cerr << "translator: unable to open input file \""
					<< dm2kGraphicRuleFile << "\"" << endl;
			return 0;
		}
		while (inf) {
			getline(inf, line);
			line = reduce(line);
			if (line.empty())
				continue;
			if (startsWithNumber(line)) {
				getTwoNumbers(line, firstNumber[numStates],
						secondNumber[numStates]);
				numStates++;
			} else {
				if (numStates > 0) {
					os << "file \"" << prefix << graphicRuleName << ".edl\"\n";
					os << "numStates " << numStates << "\n";
					os << "minValues {" << "\n";
					//if(firstNumber[0][0] == '0') {
					for (int i = 0; i < numStates; i++) {
						os << i << " " << firstNumber[i] << "\n";
					}
					//} else {
					//	for(int i = 0; i < numStates; i++) {
					//		os << i << " " << firstNumber[i] << "\n";
					//	}
					//}
					os << "}" << "\n";
					os << "maxValues {" << "\n";
					for (int i = 0; i < numStates; i++) {
						os << i << " " << secondNumber[i] << "\n";
					}
					os << "}";
					if ((graphicRuleMap[graphicRuleName]).length()) {
						cerr << graphicRuleName << " is already known" << endl;
					}
					graphicRuleMap[graphicRuleName] = os.str();
					os.str("");
					os.clear();
					//cerr << graphicRuleMap[graphicRuleName] << endl;
				}
				numStates = 0;
				getGraphicRuleName(line, graphicRuleName);
			}
		}
		if (numStates > 0) {
			os << "file \"" << prefix << graphicRuleName << ".edl\"" << "\n";
			os << "numStates " << numStates << "\n";
			os << "minValues {" << "\n";
			//if(firstNumber[0][0] == '0') {
			for (int i = 0; i < numStates; i++) {
				os << i << " " << firstNumber[i] << "\n";
			}
			//} else {
			//	for(int i = 0; i < numStates; i++) {
			//		os << i << " " << firstNumber[i] << "\n";
			//	}
			//}
			os << "}" << "\n";
			os << "maxValues {" << "\n";
			for (int i = 0; i < numStates; i++) {
				os << i << " " << secondNumber[i] << "\n";
			}
			os << "}" << "\n";
			if ((graphicRuleMap[graphicRuleName]).length()) {
				cerr << graphicRuleName << " is already known" << endl;
			}
			graphicRuleMap[graphicRuleName] = os.str();
			os.str("");
			os.clear();
			//cerr << graphicRuleMap[graphicRuleName] << endl;
		}
		inf.close();
		//cerr << "graphicRuleMap contains " << graphicRuleMap.size() << " elements\n";
		return 1;
	}
};

int main(int argc, char **argv) {
	char outfile[200];
	char *dm2kGraphicRuleFile;
	char *edmkGraphicRulePrefix;
	char emptyString[1] = "";
	urgb = false;
	retitle = false;
	int numFiles;
	int nextOption;
	Dm2kGraphicRuleParser graphicRuleParser;

	cerr << "*************** adl2edl version " << version << " ***************"
			<< endl;
	while ((nextOption = getopt(argc, argv, "hd:r:f:")) != -1) {
		// printf( "Checking option %c\n", (char) nextOption );
		switch (nextOption) {
		case 'f':
			// Get the font filename
			fontFileName = optarg;
			// printf( "Setting fontFileName = %s\n", fontFileName );
			break;
		case 'r':
			if (strncmp(optarg, "gb", 2) == 0) {
				urgb = true;
				// printf( "Setting urgb = true\n" );
			} else if (strncmp(optarg, "ename", 5) == 0) {
				retitle = true;
				// printf( "Setting retitle = true\n" );
			} else {
				cerr << "Unsupported option: -r" << optarg << endl;
				usage();
				return -1;
			}
			break;
		case 'd':
			// printf( "Checking optarg %s\n", optarg );
			if (sscanf(optarg, "%d", &debugAdl2Edl) != 1) {
				cerr << "Invalid debug level: " << optarg << endl;
				usage();
				return -1;
			}
			if (debugAdl2Edl != 0)
				printf("Setting debugAdl2Edl = %d\n", debugAdl2Edl);
			break;
		case 'h':
		default:
			usage();
			return 0;
		case '?':
			cerr << "Unsupported option: -" << (char) optopt << endl;
			usage();
			return -1;
		}
	}

	// 2 0	t *.adl
	// 3 1	t -rgb *.adl
	// 3 0	t *.adl *.edl
	// 4 1 	t -rgb *.adl *.edl
	// 4 1 	t -rename *.adl *.edl
	// 4 2 	t -rename *.adl
	// 5 2  t -rgb -rename *.adl *.edl

	numFiles = argc - optind;
	if (numFiles == 1) {
		// If user has named only one file, use the inputfilename + ".db"
		// for diagnostic output.
		strncpy(outfile, argv[optind], 196);
		strcat(outfile, ".db");
	} else if (numFiles == 2) {
		strncpy(outfile, argv[optind + 1], 196);
	} else {
		cerr << "Invalid filename specifications" << endl;
		usage();
		return -1;
	}
	if (debugAdl2Edl > 1) {
		cout << "f1: " << argv[optind] << endl;
		cout << "f2: " << outfile << endl;
	}
	edmkGraphicRulePrefix = getenv("EDM_GRAPHIC_RULE_PREFIX");
	dm2kGraphicRuleFile = getenv("DM2K_GRAPHIC_RULE");
	if (dm2kGraphicRuleFile && *dm2kGraphicRuleFile) {
		if (edmkGraphicRulePrefix && *edmkGraphicRulePrefix) {
			graphicRuleParser.parse(dm2kGraphicRuleFile, edmkGraphicRulePrefix);
		} else {
			//cerr << "environment variable EDM_GRAPHIC_RULE_PREFIX not set"
			//		<< endl;
			graphicRuleParser.parse(dm2kGraphicRuleFile, emptyString);
		}
	} else {
		cerr << "missing environment variable DM2K_GRAPHIC_RULE" << endl;
	}
	// Initialize fontInfoClass
	fi.InitializeXt();

	char *dup0;
	string dir0;
	dup0 = strdup(argv[optind]);
	dir0 = dirname(dup0);
	translator::dir = dir0;

	translator *trans;
	trans = new translator();

	int result = trans->processFile(argv[optind], outfile);
	return 0;  // processFile always returns 1
}

cmapclass::cmapclass() {
}

cmapclass::~cmapclass() {
}

string& cmapclass::getRGB(int index) {
	static string defaultColor("-1 -1 -1");

	if (index >= 0 && index < (int) cmap.rgbvec.size()) {
		//cout << "INDEX: " << index << " NC: " << cmap.numColors << endl;
		//cout << "STRING: " << cmap.rgbvec[index] << endl;
		return cmap.rgbvec[index];
	}

	else {
		//cerr << " Invalid index " << index << " NC " << cmap.numColors << endl;
		return defaultColor;
	}

}

int cmapclass::parse(ifstream &inf, ostream &outf, ostream &outd) {
	int eq_pos;
	string eq("=");
	string space(" ");
	char t[4];
	int ctr = 0;

	//outd << "In Cmap " << translator::line_ctr << endl;
	do {
		getline(inf, line);
		//outd << "L " << line << endl;
		translator::line_ctr++;
		pos = line.find(bopen, 0);
		if (pos != -1)
			open++;
		pos = line.find(bclose, 0);
		if (pos != -1)
			open--;

		eq_pos = line.find(eq, 0);
		if (eq_pos != -1) {
			line.replace(eq_pos, 1, space);
			snum = sscanf(line.c_str(), "%s %s", s1, s2);
			if (strstr(s1, "ncolors") != 0)
				numColors = atoi(s2);
		} else if (strstr(line.c_str(), "colors") != 0) {
			//outd << "In colors " << translator::line_ctr << endl;
			int done = 0;
			do {
				getline(inf, line);
				translator::line_ctr++;

				pos = line.find(bclose, 0);
				if (pos != -1) {
					done = 1;
					open--;
				} else {
					ctr++;
					sscanf(line.c_str(), "%s", s1);
					string rgb;
					int r, g, b;
					char tstr[100];

					t[0] = s1[0];
					;
					t[1] = s1[1];
					;
					t[2] = 0x0;
					r = strtol(t, 0, 16) << 8;

					t[0] = s1[2];
					;
					t[1] = s1[3];
					;
					g = strtol(t, 0, 16) << 8;

					t[0] = s1[4];
					;
					t[1] = s1[5];
					;
					b = strtol(t, 0, 16) << 8;

					//outd << r << " " << g << " " << b << endl;
					sprintf(tstr, "%d %d %d", r, g, b);
					rgb = tstr;
					cmap.rgbvec.push_back(rgb);
				}

			} while (!done);
			string rgb = "-1 -1 -1"; //One more to be used as a invalid index return
			cmap.rgbvec.push_back(rgb);
		}
	} while (open > 0);
	//outd << "numColors " << numColors << " ctr " << ctr << endl;

	// adl files contain "display {" which includes clr and bclr default values.
	// We don't have rgb conversion info until we read the cmap section which
	// follows the display section.
	// So here, we finish writing the edl screen properties.
	// We also choose to set some other default colors for edm.
	if (urgb) {
		outf << "fgColor rgb " << cmap.getRGB(default_clr) << endl;
		outf << "bgColor rgb " << cmap.getRGB(default_bclr) << endl;
		outf << "textColor rgb " << cmap.getRGB(14) << endl;
		outf << "ctlFgColor1 rgb " << cmap.getRGB(30) << endl;
		outf << "ctlFgColor2 rgb " << cmap.getRGB(32) << endl;
		outf << "ctlBgColor1 rgb " << cmap.getRGB(34) << endl;
		outf << "ctlBgColor2 rgb " << cmap.getRGB(35) << endl;
		outf << "topShadowColor rgb " << cmap.getRGB(37) << endl;
		outf << "botShadowColor rgb " << cmap.getRGB(44) << endl;
	} else {
		outf << "fgColor index " << default_clr << endl;
		outf << "bgColor index " << default_bclr << endl;
		outf << "textColor index 14" << endl;
		outf << "ctlFgColor1 index 25" << endl;
		outf << "ctlFgColor2 index 15" << endl;
		outf << "ctlBgColor1 index 5" << endl;
		outf << "ctlBgColor2 index 12" << endl;
		outf << "topShadowColor index 1" << endl;
		outf << "botShadowColor index 13" << endl;
	}

	outf << "showGrid" << endl;
	outf << "snapToGrid" << endl;
	outf << "gridSize 4" << endl;
	outf << "endScreenProperties" << endl;
	return 1;

}

headclass::headclass() {
}

headclass::~headclass() {
}

translator::translator() {
	translator::line_ctr = 0;
	translator::display_height = 0;
	translator::display_width = 0;
}

translator::~translator() {
}

// Open argv[1] file and get version
int translator::processFile(char *in, char *out) {
	string line;
	int comp_flag = 0;
	bool normal_mode;

	// input file
	ifstream inf(in);
	if (!inf) {
		cerr << "translator: unable to open input file \"" << in << "\""
				<< endl;
		exit(1);
	}

	// output file
	ofstream toutf(out);
	if (!toutf) {
		cerr << "translator:unable to open output file \"" << out << "\""
				<< endl;
		exit(1);
	}

	if (strstr(out, ".db") != 0x0)
		normal_mode = false;
	else
		normal_mode = true;

	// if normal_mode, write to the output file
	// else send output to stdout for piping to edm
	ostream *outf = (normal_mode) ? (ostream*) &toutf : (ostream*) &cout;

	// if normal_mode, send diags to stdout
	// else diags go to the file "filename.adl.db"
	ostream *outd = (!normal_mode) ? (ostream*) &toutf : (ostream*) &cout;
	*outd << endl << "************************ parse file " << in << " **********************" << endl;
	// open temp file for bubbling control widgets
	char tfilename[256];
	strcpy(tfilename, "adltemp_XXXXXX");
	int temp_fd = mkstemp(tfilename);
	if (temp_fd == -1) {
		printf("Error calling mkstemp for temporary file %s\n", tfilename);
		exit(1);
	}
	ofstream outt(tfilename);
	if (!outt) {
		printf("Error creating temporary output file %s\n", tfilename);
		exit(1);
	}

	// So we can write to 3 files:
	// outf - data
	// outd - debug
	// outt - bubble control data

	int use_battr = 0;
	int use_dattr = 0;
	size_t space_pos = 0;
	size_t pos;
	string space(" ");
	string bopen("{");
	string bclose("}");
	int open = 0;
	int snum;
	char s1[256], s2[256];
	int tpos;
	string chan;
	string squote;
	squote = "\"";
	string nil("");
	int attr = 0;

	while (inf) {
		getline(inf, line);
//		*outd << line << endl;
		translator::line_ctr++;
		pos = line.find(bopen, 0);
		if ((int) pos != -1) {
			open++;
			if (open == 1) {
				snum = sscanf(line.c_str(), "%s %s", s1, s2);
				if (!strcmp(s1, "display")) {
					headclass head;
					head.parse(inf, *outf, *outd);
					open--;
				} else if (strstr(s1, "shell") != 0x0) {
					attr = use_battr + 2 * use_dattr;
					shellclass shell(attr);
					use_dattr = 0;
					//shell.parse(inf, outt, *outd);
					shell.parse(inf, *outf, *outd);
					open--;
				} else if (strstr(s1, "message") != 0x0) {
					if (strstr(s2, "button") != 0x0) {
						attr = use_battr + 2 * use_dattr;
						mbuttonclass mbutton(attr);
						use_dattr = 0;
						mbutton.parse(inf, outt, *outd);
						//mbutton.parse(inf, *outf, *outd);
						open--;
					}
				} else if (strstr(s1, "choice") != 0x0) {
					if (strstr(s2, "button") != 0x0) {
						attr = use_battr + 2 * use_dattr;
						buttonclass button(attr);
						use_dattr = 0;
						button.parse(inf, outt, *outd);
						//button.parse(inf, *outf, *outd);
						open--;
					}
				} else if (strstr(s1, "related") != 0x0) {
					attr = use_battr + 2 * use_dattr;
					relatedclass related(attr);
					use_dattr = 0;
					related.parse(inf, outt, *outd);
					//related.parse(inf, *outf, *outd);
					open--;
				} else if (strstr(s1, "menu") != 0x0) {
					attr = use_battr + 2 * use_dattr;
					menuclass menu(attr);
					use_dattr = 0;
					menu.parse(inf, *outf, *outd);
					open--;
				} else if (strstr(s1, "byte") != 0x0) {
					attr = use_battr + 2 * use_dattr;
					byteclass byte(attr);
					use_dattr = 0;
					byte.parse(inf, *outf, *outd);
					open--;
				} else if (strstr(s1, "text") != 0x0) {
					attr = use_battr + 2 * use_dattr;
					if (strstr(s2, "update") != 0x0) {
						textmonclass textmon(attr);
						use_dattr = 0;
						textmon.parse(inf, *outf, *outd, 0);
					} else if (strstr(s2, "entry")) {
						textmonclass textmon(attr);
						use_dattr = 0;
						textmon.parse(inf, *outf, *outd, 1);
					} else {
						textclass text(attr);
						use_dattr = 0;
						text.parse(inf, *outf, *outd);
					}
					open--;
				} else if (!strcmp(s1, "rectangle")) {
					attr = use_battr + 2 * use_dattr;
					rectclass rect(attr);
					use_dattr = 0;
					rect.parse(inf, *outf, *outd);
					open--;
				} else if (!strcmp(s1, "arc")) {
					attr = use_battr + 2 * use_dattr;
					arcclass arc(attr);
					use_dattr = 0;
					arc.parse(inf, *outf, *outd);
					open--;
				} else if (!strcmp(s1, "oval")) {
					attr = use_battr + 2 * use_dattr;
					circleclass circle(attr);
					use_dattr = 0;
					circle.parse(inf, *outf, *outd);
					open--;
				} else if (!strcmp(s1, "meter")) {
					attr = use_battr + 2 * use_dattr;
					meterclass meter(attr);
					use_dattr = 0;
					meter.parse(inf, *outf, *outd);
					open--;
				} else if (!strcmp(s1, "bar") || !strcmp(s1, "indicator")) {
					attr = use_battr + 2 * use_dattr;
					barclass bar(attr);
					use_dattr = 0;
					bar.parse(inf, *outf, *outd);
					open--;
				} else if (!strcmp(s1, "valuator")) {
					attr = use_battr + 2 * use_dattr;
					valclass val(attr);
					use_dattr = 0;
					val.parse(inf, *outf, *outd);
					open--;
				} else if (strstr(s1, "poly") != 0x0) {
					attr = use_battr + 2 * use_dattr;
					polyclass poly(attr);
					use_dattr = 0;
					if (strstr(s1, "polyline") != 0x0)
						poly.parse(inf, *outf, *outd, 0);
					else
						poly.parse(inf, *outf, *outd, 1);
					open--;
				} else if (strstr(s1, "composite") != 0x0) {
					parseComposite(inf, *outf, *outd, outt);
					comp_flag += 2;
					open--;
				} else if (strstr(s1, "cartesian") != 0x0) {
					attr = use_battr + 2 * use_dattr;
					xyclass xy(attr);
					use_dattr = 0;
					xy.parse(inf, *outf, *outd);
					open--;
				} else if (strstr(s1, "strip") != 0x0) {
					attr = use_battr + 2 * use_dattr;
					stripclass strip(attr);
					use_dattr = 0;
					strip.parse(inf, *outf, *outd);
					open--;
				}
				// These next two cases are for either very old MEDM or
				// for jlab's programatic screen builder output.
				else if (strstr(line.c_str(), "basic") != 0x0) {
					use_battr = read_battr(inf, *outd);
					open--;
				} else if (strstr(line.c_str(), "dynamic symbol") != 0x0) {
					attr = use_battr + 2 * use_dattr;
					dsymclass dsym(attr);
					use_dattr = 0;
					dsym.parse(inf, *outf, *outd);
					open--;
				} else if (!strcmp(s1, "image")) {
					attr = use_battr + 2 * use_dattr;
					gifclass gifimage(attr);
					use_dattr = 0;
					gifimage.parse(inf, *outf, *outd);
					open--;
				} else if (strstr(line.c_str(), "dynamic") != 0x0) {
					use_dattr = read_dattr(inf, *outd);
					open--;
				}

				else if (strstr(line.c_str(), "file") != 0x0)
					open--;
				else if (strstr(line.c_str(), "color map") != 0x0) {
					cmap.parse(inf, *outf, *outd);
					open--;
				} else {
					*outd << "Main NOT found: " << translator::line_ctr << line
							<< endl;
					open--;
				}
			} // end if we're in a bopen

		} else {
			pos = line.find(bclose, 0);
			if ((int) pos != -1 && comp_flag) {
				parseCompositeClose(inf, *outf, *outd, outt);
				comp_flag -= 2;
			}
		}
	}
	//*outd <<  "end open: " << open <<  " " << translator::line_ctr << endl;
	//*outf <<  "end open: " << open <<  " " << translator::line_ctr << endl;
	inf.close();

	// open temp file for bubble control widgets
	ifstream in_t(tfilename);
	if (!in_t) {
		printf("Error creating temporary input file %s\n", tfilename);
		exit(1);
	}

	int ctr = 0;
	while (in_t) {
		getline(in_t, line);
		*outf << line << endl;
		ctr++;
	}
	in_t.close();
	outt.close();
	close(temp_fd);
	remove(tfilename);

	return 1;
}

int translator::read_battr(ifstream &inf, ostream &outd) {
	string line;
	int pos;
	int eq_pos;
	int open = 1;
	string bopen("{");
	string bclose("}");
	string space(" ");
	string eq("=");
	int snum;
	char s1[256], s2[256];

	//outd << "In read_battr " << translator::line_ctr << endl;

	do {
		getline(inf, line);
		translator::line_ctr++;
		//outd << "L " << line << " " << translator::line_ctr << endl;
		pos = line.find(bopen, 0);
		if (pos != -1)
			open++;
		pos = line.find(bclose, 0);
		if (pos != -1)
			open--;

		eq_pos = line.find(eq, 0);
		if (eq_pos != -1) {
			line.replace(eq_pos, 1, space);
			snum = sscanf(line.c_str(), "%s %s", s1, s2);

			if (!strcmp(s1, "clr"))
				translator::ba.clr = atoi(s2);
			else if (!strcmp(s1, "style")) {
				if (strstr(s2, "solid") != 0x0)
					translator::ba.style = 0;
				else
					ba.style = 1;
			} else if (!strcmp(s1, "fill")) {
				//if(strstr(s2, "solid")!= 0x0) translator::ba.fill = 1;
				//else  ba.fill = 0;
				if (strstr(s2, "outline") != 0x0)
					translator::ba.fill = 0;
				else
					ba.fill = 1;
			} else if (!strcmp(s1, "width"))
				translator::ba.width = atoi(s2);
		}
	} while (open > 0);
	return 1;
}

int translator::read_dattr(ifstream &inf, ostream &outd) {
	string line;
	int pos;
	int eq_pos;
	int open = 1;
	string bopen("{");
	string bclose("}");
	string space(" ");
	string eq("=");
	int snum;
	char s1[256], s2[256];
	int x, y, wid, hgt;

	//outd << "In read_dattr " << translator::line_ctr << endl;

	do {
		getline(inf, line);
		//outd << "L " << line << endl;
		translator::line_ctr++;
		pos = line.find(bopen, 0);
		if (pos != -1)
			open++;
		pos = line.find(bclose, 0);
		if (pos != -1)
			open--;

		eq_pos = 0;
		while (eq_pos != -1) {
			eq_pos = line.find(eq, 0);
			if (eq_pos != -1) {
				line.replace(eq_pos, 1, space);
				snum = sscanf(line.c_str(), "%s %s", s1, s2);
				if (!strcmp(s1, "clr")) {
					if (strstr(s2, "static") != 0x0)
						translator::da.colormode = 0;
					else if (strstr(s2, "alarm") != 0x0)
						translator::da.colormode = 1;
					else if (strstr(s2, "discrete") != 0x0)
						translator::da.colormode = 2;
				} else if (strstr(s1, "vis") != 0x0) {
					if (strstr(line.c_str(), "static") != 0x0)
						translator::da.vis = 0;
					else if (strstr(line.c_str(), "if not zero") != 0x0)
						translator::da.vis = 1;
					else if (strstr(line.c_str(), "if zero") != 0x0)
						translator::da.vis = 2;
					else if (strstr(line.c_str(), "calc") != 0x0)
						translator::da.vis = 3;
				} else if (!strcmp(s1, "calc"))
					translator::da.calc = s2;
				else if (!strcmp(s1, "chan"))
					translator::da.chan = s2;
				else if (!strcmp(s1, "chanB"))
					translator::da.chanB = s2;
				else if (!strcmp(s1, "chanC"))
					translator::da.chanC = s2;
				else if (!strcmp(s1, "chanD"))
					translator::da.chanD = s2;
			}
		}

	} while (open > 0);
	return 1;
}

void comclass::fill_attrs(int attr) {
	if (!attr) {
		fill = 1;       // 0=solid,1=filled in medm. in edm default is filled=1;
		style = 0;          // solid, not dashed, line
		linewidth = 0;
		colormode = 0;		//0=static,1=alarm,2=discrete
		vis = 0;            // static, if not zero, if zero, calc
		visInverted = 1;
	} else {
		if (attr & 1) {
			clr = translator::ba.clr;
			style = translator::ba.style;
			fill = translator::ba.fill;
			linewidth = translator::ba.width;
			colormode = 0;		//0=static,1=alarm,2=discrete
			vis = 0;            // static, if not zero, if zero, calc
			visInverted = 1;
		}
		if (attr & 2) {
			colormode = translator::da.colormode;
			vis = translator::da.vis;
			calc = translator::da.calc;
			chan = translator::da.chan;
			chanB = translator::da.chanB;
			chanC = translator::da.chanC;
			chanD = translator::da.chanD;
		}
	}
}

int translator::parseComposite(ifstream &inf, ostream &outf, ostream &outd,
		ostream &outt) {
	string line;
	int pos;
	int eq_pos;
	int tpos;
	int open = 1;
	string bopen("{");
	string bclose("}");
	string eq("=");
	string space(" ");
	string squote = "\"";
	string nil("");
	int snum;
	char s1[256], s2[256];
	int x = 0, y = 0, wid = 0, hgt = 0;
	string file;
	int adl_pos;
	string echar(".edl");
	string adl(".adl");
	bool group = false;	// true=group, false=embedded window
	bool menu = false;
	int vis = 0;
	string calc;
	string chan;
	int attr = 0;
	int use_dattr = 0;
	int use_battr = 0;
	vector<shellnode> menu_items;
	shellnode *menu_item = NULL;

	//outd << "In Composite " << translator::line_ctr << endl;
	do {
		getline(inf, line);
		//outd << "L " << line << endl;
		translator::line_ctr++;
		pos = line.find(bopen, 0);
		if (pos != -1)
			open++;
		pos = line.find(bclose, 0);
		if (pos != -1) {
			if (menu) {
				menu_items.push_back(*menu_item);
				if(menu_item) {
					delete menu_item;
					menu_item = NULL;
				}
				menu = false;
			}
			open--;
		}

		eq_pos = line.find(eq, 0);
		if (eq_pos != -1) {
			sscanf(line.substr(0, eq_pos).c_str(), "%s", s1);
			sscanf(line.substr(eq_pos + 1).c_str(), "%s", s2);
			if (menu) {
				if (!strcmp(s1, "label")) {
					menu_item->label = string(line, eq_pos, std::string::npos);
				} else if (!strcmp(s1, "type")) {
					menu_item->type = s2;
				} else if (!strcmp(s1, "command")) {
					menu_item->command = s2;
				}  else if (!strcmp(s1, "args")) {
					menu_item->args = s2;
				} else {
					outd << "MenuItem: Can't decode line: " << translator::line_ctr << line << endl;
				}
			} else {
				if (!strcmp(s1, "x")) {
					x = atoi(s2);
				} else if (!strcmp(s1, "y")) {
					y = atoi(s2);
				} else if (!strcmp(s1, "width")) {
					wid = atoi(s2);
				} else if (!strcmp(s1, "height")) {
					hgt = atoi(s2);
				} else if (strstr(s1, "vis") != 0x0) {
					if (strstr(line.c_str(), "static") != 0x0) {
						vis = 0;
					} else if (strstr(line.c_str(), "if not zero") != 0x0) {
						vis = 1;
					} else if (strstr(line.c_str(), "if zero") != 0x0) {
						vis = 2;
					} else if (strstr(line.c_str(), "calc") != 0x0) {
						vis = 3;
					}
				} else if (!strcmp(s1, "calc")) {
					// calc "A=2"
					while ((tpos = line.find(squote, 0)) != -1) {
						line.replace(tpos, 1, nil);
					}
					// calc A=2
					eq_pos = line.find(eq, 0);
					calc = string(line, eq_pos + 1, line.length());
					// 2
				} else if (!strcmp(s1, "chan")) {
					chan = s2;
				} else if (strstr(line.c_str(), "composite name") != 0x0) {
					; // ignore
				} else if (strstr(line.c_str(), "composite file") != 0x0) {
					group = false;
					string slash("/");
					int slash_pos;
					slash_pos = line.rfind(slash);
					if (slash_pos != (int) std::string::npos) {
						file.append(line, slash_pos + 1, std::string::npos);
					} else {
						file = s2;
					}
					while ((tpos = file.find(squote, 0)) != -1) {
						file.replace(tpos, 1, nil);
					}
					adl_pos = file.find(adl, 0);
					if (adl_pos != -1)
						file.replace(adl_pos, 4, echar);
				} else {
					outd << "Translator: Can't decode line: "
							<< translator::line_ctr << line << endl;
				}
			}
		} else { // Begin no eq_pos
			if (strstr(line.c_str(), "children") != 0x0) {
				group = true;
			} else if (strstr(line.c_str(), "menuItem") != 0x0) {
				if(menu_item) {
					delete menu_item;
					menu_item = NULL;
				}
				menu_item = new shellnode();
				menu = true;
			}
		}
	} while (open > 0 && group == 0);

	if (vis) {
		if (calc.length() == 0)
			calc = "A";
		while ((tpos = calc.find(squote, 0)) != -1)
			calc.replace(tpos, 1, nil);
		while ((tpos = chan.find(squote, 0)) != -1)
			chan.replace(tpos, 1, nil);
	}
	outf << endl;

	/// Three possibilities:
	// 1=group						(group=true)
	// 2=embed win					(group=false, vis = 0)
	// 3=embed win with vis param	(group=false, vis > 0)

	if (group) {
		//outd << "CASE 1 GROUP" << endl;
		outf << "# (Group)" << endl;
		outf << "object activeGroupClass" << endl;
		outf << "beginObjectProperties" << endl;
		outf << "major " << AGC_MAJOR_VERSION << endl;
		outf << "minor " << AGC_MINOR_VERSION << endl;
		outf << "release " << AGC_RELEASE << endl;
		outf << "x " << x << endl;
		outf << "y " << y << endl;
		outf << "w " << wid << endl;
		outf << "h " << hgt << endl;
		if(x < -wid || x > translator::display_width || y < -hgt || y > translator::display_height)
		    outd << "Group: Not visible on display!" << " at " << translator::line_ctr << endl;
		outf << endl;
		outf << "beginGroup" << endl;
		outf << endl;
	} else if (!vis) {
		//outd << "CASE 2 embed win" << endl;
		outf << "# (Embedded Window)" << endl;
		outf << "object activePipClass" << endl;
		outf << "beginObjectProperties" << endl;
		outf << "major " << AGC_MAJOR_VERSION << endl;
		outf << "minor " << AGC_MINOR_VERSION << endl;
		outf << "release " << AGC_RELEASE << endl;
		outf << "x " << x << endl;
		outf << "y " << y << endl;
		outf << "w " << wid << endl;
		outf << "h " << hgt << endl;
		if(x < -wid || x > translator::display_width || y < -hgt || y > translator::display_height)
		    outd << "Embedded Window: Not visible on display!" << " at " << translator::line_ctr << endl;
		if (urgb) {
			outf << "fgColor rgb " << cmap.getRGB(14) << endl;
			outf << "bgColor rgb " << cmap.getRGB(0) << endl;
			outf << "topShadowColor rgb " << cmap.getRGB(0) << endl;
			outf << "botShadowColor rgb " << cmap.getRGB(14) << endl;
		} else {
			outf << "fgColor index 14" << endl;
			outf << "bgColor index 0" << endl;
			outf << "topShadowColor index 0" << endl;
			outf << "botShadowColor index 14" << endl;
		}
		outf << "displaySource \"file\"" << endl;
		outf << "file \"" << file << "\"" << endl;
		outf << "sizeOfs 5" << endl;
		outf << "numDsps 1" << endl;
		outf << "displayFileName {" << endl;
		outf << "  " << 0 << " \"" << file << "\"" << endl;
		outf << "}" << endl;
		outf << "noScroll" << endl;
		outf << "endObjectProperties" << endl;
	} else {
		//outd << "CASE 3 embed win with vis param" << endl;
		outf << "# (Group)" << endl;
		outf << "object activeGroupClass" << endl;
		outf << "beginObjectProperties" << endl;
		outf << "major " << AGC_MAJOR_VERSION << endl;
		outf << "minor " << AGC_MINOR_VERSION << endl;
		outf << "release " << AGC_RELEASE << endl;
		outf << "x " << x << endl;
		outf << "y " << y << endl;
		outf << "w " << wid << endl;
		outf << "h " << hgt << endl;
		if(x < -wid || x > translator::display_width || y < -hgt || y > translator::display_height)
		    outd << "Group: Not visible on display!" << " at " << translator::line_ctr << endl;
		outf << endl;
		outf << "beginGroup" << endl;
		outf << endl;

		outf << "# (Embedded Window)" << endl;
		outf << "object activePipClass" << endl;
		outf << "beginObjectProperties" << endl;
		outf << "major " << PIP_MAJOR_VERSION << endl;
		outf << "minor " << PIP_MINOR_VERSION << endl;
		outf << "release " << PIP_RELEASE << endl;
		outf << "x " << x << endl;
		outf << "y " << y << endl;
		outf << "w " << wid << endl;
		outf << "h " << hgt << endl;
		if(x < -wid || x > translator::display_width || y < -hgt || y > translator::display_height)
		    outd << "Embedded Window: Not visible on display!" << " at " << translator::line_ctr << endl;
		if (urgb) {
			outf << "fgColor rgb " << cmap.getRGB(14) << endl;
			outf << "bgColor rgb " << cmap.getRGB(0) << endl;
			outf << "topShadowColor rgb " << cmap.getRGB(0) << endl;
			outf << "botShadowColor rgb " << cmap.getRGB(14) << endl;
		} else {
			outf << "fgColor index 14" << endl;
			outf << "bgColor index 0" << endl;
			outf << "topShadowColor index 0" << endl;
			outf << "botShadowColor index 14" << endl;
		}
		outf << "displaySource \"file\"" << endl;
		outf << "file \"" << file << "\"" << endl;
		outf << "sizeOfs 5" << endl;
		outf << "numDsps 1" << endl;
		outf << "displayFileName {" << endl;
		outf << "  " << 0 << " \"" << file << "\"" << endl;
		outf << "}" << endl;
		outf << "noScroll" << endl;
		outf << "endObjectProperties" << endl;

		outf << endl;
		outf << "endGroup" << endl;
		outf << endl;
		/*
		 string tstr = "visPv \"CALC\\\\\\{(";
		 string tstr2 = ")\\}(";
		 outf <<  tstr << calc << tstr2 << chan ;
		 outf << ")\"" << endl;
		 */
		if (chan.length() > 0) {
			outf << "visPv " << "\"" << chan << "\"" << endl;
		}
		outf << "visMin " << "\"" << calc << "\"" << endl;
		outf << "visMax " << "\"" << atoi(calc.c_str()) + 1 << "\"" << endl;

		outf << "endObjectProperties" << endl;
	}
	if(menu_items.size()) {
		outf << "# (Shell Command)" << endl;
		outf << "object shellCmdClass" << endl;
		outf << "beginObjectProperties" << endl;
		outf << "major " << AGC_MAJOR_VERSION << endl;
		outf << "minor " << AGC_MINOR_VERSION << endl;
		outf << "release " << AGC_RELEASE << endl;
		outf << "x " << x << endl;
		outf << "y " << y << endl;
		outf << "w " << wid << endl;
		outf << "h " << hgt << endl;
		if(x < -wid || x > translator::display_width || y < -hgt || y > translator::display_height)
		    outd << "Shell Command: Not visible on display!" << " at " << translator::line_ctr << endl;
		if (urgb) {
			outf << "fgColor rgb " << cmap.getRGB(14) << endl;
			outf << "bgColor rgb " << cmap.getRGB(0) << endl;
			outf << "topShadowColor rgb " << cmap.getRGB(0) << endl;
			outf << "botShadowColor rgb " << cmap.getRGB(14) << endl;
		} else {
			outf << "fgColor index 14" << endl;
			outf << "bgColor index 0" << endl;
			outf << "topShadowColor index 0" << endl;
			outf << "botShadowColor index 14" << endl;
		}
		if(menu_items.size() > 1) {
			outf << "invisible" << endl;
		}
		outf << "numCmds " << menu_items.size() << endl;
		outf << "commandLabel {" << endl;
		for(int i=0; i < (int)menu_items.size(); i++) {
			outf << i << " " << menu_items[i].label << endl;
		}
		outf << "}" << endl;
		outf << "command {" << endl;
		for(int i=0; i < (int)menu_items.size(); i++) {
			outf << i << " " << menu_items[i].command << endl;
		}
		outf << "}" << endl;
		outf << "endObjectProperties" << endl;
		delete menu_item;
		menu_item = NULL;
	}
	menu_items.clear();
	return 0;
}

// process the "display" object
int headclass::parse(ifstream &inf, ostream &outf, ostream &outd) {
	//outd << "In Header " <<  translator::line_ctr << endl;
	do {
		getline(inf, line);
		translator::line_ctr++;
		pos = line.find(bopen, 0);
		if (pos != -1)
			open++;
		pos = line.find(bclose, 0);
		if (pos != -1)
			open--;

		eq_pos = 0;
		while (eq_pos != -1) {
			eq_pos = line.find(eq, 0);
			if (eq_pos != -1) {
				line.replace(eq_pos, 1, space);
				snum = sscanf(line.c_str(), "%s %s", s1, s2);
				if (!strcmp(s1, "x"))
					x = atoi(s2);
				else if (!strcmp(s1, "y"))
					y = atoi(s2);
				else if (!strcmp(s1, "width"))
					wid = atoi(s2);
				else if (!strcmp(s1, "height"))
					hgt = atoi(s2);
				else if (!strcmp(s1, "clr"))
					default_clr = atoi(s2);
				else if (!strcmp(s1, "bclr"))
					default_bclr = atoi(s2);
				else if (!strcmp(s1, "gridSpacing"))
					gridSpacing = atoi(s2);
				else if (!strcmp(s1, "gridOn")) {
					gridOn = atoi(s2);
					gridShow = atoi(s2);
				} else if (!strcmp(s1, "snapToGrid")) {
					snapToGrid = atoi(s2);
				} else if (!strcmp(s1, "cmap")) {
				} // do nothing
				else {
					outd << "Header: Can't decode header line: " << translator::line_ctr
							<< line << endl;
				}
			}
		}
	} while (open > 0);

	outf << AWC_MAJOR_VERSION << " " << AWC_MINOR_VERSION << " " << AWC_RELEASE
			<< endl;
	outf << "beginScreenProperties" << endl;
	outf << "major " << AWC_MAJOR_VERSION << endl;
	outf << "minor " << AWC_MINOR_VERSION << endl;
	outf << "release " << AWC_RELEASE << endl;
	outf << "x " << x << endl;
	outf << "y " << y << endl;
	outf << "w " << wid << endl;
	translator::display_width = wid;
	outf << "h " << hgt << endl;
	translator::display_height = hgt;

	fptr = fi.bestFittingFont(25);
	if (fptr) {
		outf << "font \"" << fptr << "\"" << endl;
	} else {
		outf << "font \"" << "helvetica-medium-r-18.0" << "\"" << endl;
	}

	fptr = fi.bestFittingFont(8);
	if (fptr) {
		outf << "ctlFont \"" << fptr << "\"" << endl;
	} else {
		outf << "ctlFont \"" << "helvetica-bold-r-10.0" << "\"" << endl;
	}

	fptr = fi.bestFittingFont(25);
	if (fptr) {
		outf << "btnFont \"" << fptr << "\"" << endl;
	} else {
		outf << "btnFont \"" << "helvetica-medium-r-18.0" << "\"" << endl;
	}

	// default colors and "endScreenProperties" will be written
	// after we read the color map.
	return 1;
}

int translator::parseCompositeClose(ifstream &inf, ostream &outf, ostream &outd,
		ostream &outt) {
	int tpos;
	int pos;
	int eq_pos;
	string chan;
	string chanB;
	string chanC;
	string chanD;
	string calc;
	string squote;
	squote = "\"";
	string nil("");
	string eq("=");
	string space(" ");
	string bclose("}");
	string bopen("{");
	string line;
	bool dyn = false;
	char s1[256], s2[256];
	int close_ct = 0;
	int colormode = 0;
	int vis = 0;
	int visInverted = 1;

	// We may get ""dynamic attribute" bopen" or bclose.
	outf << endl;
	outt << endl;
	// get dynamic comp attrs
	while (1) {
		getline(inf, line);
		if (inf.eof())
			break;
		//outd << line << endl;
		translator::line_ctr++;
		pos = line.find(bclose, 0);
		if (pos != -1) {
			// We got a close curly brace. This means either there were no
			// dynamic attrs or this is our second time here and we're
			// closing dyn attrs.
			if (close_ct == 0) {
				//outd << "END GROUP" << endl;
				break;
			}
			close_ct--;
		}
		if ((tpos = line.find(bopen, 0)) != -1) { // We have dyn attrs
			dyn = true;
			close_ct = 1;
		} else {
			eq_pos = line.find(eq, 0);
			if (eq_pos != -1) {
				line.replace(eq_pos, 1, space);
				sscanf(line.c_str(), "%s %s", s1, s2);
				if (strstr(s1, "vis") != 0x0) {
					if (strstr(line.c_str(), "static") != 0x0)
						vis = 0;
					else if (strstr(line.c_str(), "if not zero") != 0x0)
						vis = 1;
					else if (strstr(line.c_str(), "if zero") != 0x0)
						vis = 2;
					else if (strstr(line.c_str(), "calc") != 0x0)
						vis = 3;
					//outd << "vis " << vis << endl;
				} else if (strstr(s1, "calc") != 0x0) {
					while ((tpos = line.find(squote, 0)) != -1) {
						line.replace(tpos, 1, nil);
					}
					calc = string(line, eq_pos + 1, (std::string::npos));
				} else if (strstr(s1, "chan") != 0x0) {
					while (1) {
						// Process the input line, stripping off everything 
						// up to the first quote.
						if ((tpos = line.find(squote, 0)) != -1)
							break;
						line.replace(0, 1, nil);
					}
					// Copy the processed line to the appropriate variable.
					if (strstr(s1, "chanB") != 0x0)
						chanB.append(line, tpos, line.length());
					else if (strstr(s1, "chanC") != 0x0)
						chanC.append(line, tpos, line.length());
					else if (strstr(s1, "chanD") != 0x0)
						chanD.append(line, tpos, line.length());
					else if (strstr(s1, "chan") != 0x0)
						chan.append(line, tpos, line.length());
				}
			}
		}
	}

	// We're using the "dyn" flag (the composite has/hasn't dynamic properties)
	// here and below rather than "vis" because some medm files may contain only
	// "chan=pvname" and nothing else (no "vis") for group dynamic attrs.

	// Strip the quotes from the pv names when used for visibility pv
	if (dyn) {
		while ((tpos = calc.find(squote, 0)) != -1)
			calc.replace(tpos, 1, nil);
		while ((tpos = chan.find(squote, 0)) != -1)
			chan.replace(tpos, 1, nil);
		while ((tpos = chanB.find(squote, 0)) != -1)
			chanB.replace(tpos, 1, nil);
		while ((tpos = chanC.find(squote, 0)) != -1)
			chanC.replace(tpos, 1, nil);
		while ((tpos = chanD.find(squote, 0)) != -1)
			chanD.replace(tpos, 1, nil);
	}
	if (dyn && calc.length() == 0)
		calc = "A";

	outf << "endGroup" << endl;
	outf << endl;

	if (dyn) { // If there were dynamic attrs
		string tstr = "visPv \"CALC\\\\\\{(";
		string tstr2 = ")\\}(";

		outf << tstr << calc << tstr2 << chan;
		if (chanB.length())
			outf << ", " << chanB;
		if (chanC.length())
			outf << ", " << chanC;
		if (chanD.length())
			outf << ", " << chanD;
		outf << ")\"" << endl;

		outt << tstr << calc << tstr2 << chan;
		if (chanB.length())
			outt << ", " << chanB;
		if (chanC.length())
			outt << ", " << chanC;
		if (chanD.length())
			outt << ", " << chanD;
		outt << ")\"" << endl;

		if (vis == 1 || vis == 3) {
			outf << "visInvert" << endl;
			outt << "visInvert" << endl;
		}
		if (vis) {
			outf << "visMin 0" << endl;
			outf << "visMax 1" << endl;
			outt << "visMin 0" << endl;
			outt << "visMax 1" << endl;
		}
	}

	outf << "endObjectProperties" << endl;
	outf << endl;
	//outd << "Done Comp" << endl;
	return 1;
}

