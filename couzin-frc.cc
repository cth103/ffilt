#include <dcp/mono_picture_asset.h>
#include <dcp/picture_asset_writer.h>
#include <getopt.h>
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <fstream>
#include <set>

using std::cout;
using std::cerr;
using std::string;
using std::set;
using std::getline;
using std::ifstream;
using boost::optional;
using boost::shared_ptr;
using boost::lexical_cast;

void
help(string n)
{
	cerr << "Syntax: " << n << " [OPTION] <MXF>\n"
	     << "  -h, --help   show this help\n"
	     << "  -f, --frames text file containing frame indices to use (first frame being 0)\n"
	     << "  -o, --output specify output MXF\n";
}

int
main(int argc, char* argv[])
{
	optional<boost::filesystem::path> output_file;
	optional<boost::filesystem::path> frames_file;

	int option_index = 0;
	while (true) {
		static struct option long_options[] = {
			{ "help", no_argument, 0, 'h'},
			{ "output", required_argument, 0, 'o' },
			{ "frames", required_argument, 0, 'f' },
			{ 0, 0, 0, 0 }
		};

		int c = getopt_long(argc, argv, "vho:f:", long_options, &option_index);

		if (c == -1) {
			break;
		}

		switch (c) {
		case 'h':
			help(argv[0]);
			exit(EXIT_SUCCESS);
		case 'o':
			output_file = optarg;
			break;
		case 'f':
			frames_file = optarg;
			break;
		}
	}

	if (optind >= argc) {
		help(argv[0]);
		exit(EXIT_FAILURE);
	}

	if (!output_file || !frames_file) {
		cerr << "You must specify -o,--output and -f,--frames\n";
		exit(EXIT_FAILURE);
	}

	set<int64_t> frames;
	ifstream frames_stream(frames_file->c_str());
	while (frames_stream.good()) {
		string line;
		getline(frames_stream, line);
		if (!line.empty()) {
			frames.insert(lexical_cast<int>(line));
		}
	}

	boost::filesystem::path const input_file = argv[optind];

	dcp::MonoPictureAsset input_asset(input_file);
	if (input_asset.frame_rate() != dcp::Fraction(50,1 )) {
		cerr << "Expected 50fps input, not " << input_asset.frame_rate().numerator << ".\n";
//		exit(EXIT_FAILURE);
	}
	shared_ptr<dcp::MonoPictureAssetReader> reader = input_asset.start_read();

	dcp::MonoPictureAsset output_asset(dcp::Fraction(48, 1));
	shared_ptr<dcp::PictureAssetWriter> writer = output_asset.start_write(*output_file, dcp::INTEROP, false);

	int written = 0;
	for (int i = 0; i < input_asset.intrinsic_duration(); ++i) {
		if (frames.find(i) != frames.end()) {
			writer->write(reader->get_frame(i)->j2k_data(), reader->get_frame(i)->j2k_size());
			++written;
		}
	}

	writer->finalize();

	cout << written << " frames written of " << input_asset.intrinsic_duration() << " total.\n";

	return 0;
}
