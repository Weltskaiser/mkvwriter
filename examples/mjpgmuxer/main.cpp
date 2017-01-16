#include <cmath>
#include <vector>
#include <fstream>
#include <iostream>
#include <iterator>

#include "mkvwriter/MatroskaMuxer.h"


using namespace mkvwriter;


void SetStreamHeader( MatroskaMuxer &muxer, int stream, int w, int h, std::string codecId )
{
    std::cout << "Writing " << codecId <<
        " (" << w << "x" << h << ") to track " << stream+1 << std::endl;

    muxer.Set_Track_CodecID( stream+1, codecId );
//  muxer.Set_Track_CodecPrivate( stream+1, data, data_size );

#if 1
    muxer.Set_Track_Video( stream+1, w, h );
#else
    muxer.Set_Track_Audio( stream+1, num_channels, sample_rate, sample_size );
#endif
}


std::vector< uint8 > ReadFrame( const char *filename )
{
    std::ifstream file( filename, std::ios::binary );
    if (!file) throw std::runtime_error( std::string( "Failed to read '" ) + filename + "'" );

    return std::vector< uint8 >(
        std::istreambuf_iterator< char >( file ),
        std::istreambuf_iterator< char >() );
}


void CopyFrames( MatroskaMuxer &muxer, int stream, float fps, char **files_begin, char **files_end )
{
    int frameCounter = 0;
    for (char **file = files_begin; file != files_end; ++file)
    {
        std::vector< uint8 > buffer = ReadFrame( *file );

        uint64 ts  = roundf( frameCounter * 1000.0f / fps );
        uint64 dur = roundf( 1000.0f / fps );
        int ref = 0; // or MatroskaMuxer::REFERENCE_PREV_FRAME
        muxer.AddFrame( stream+1, ts, dur, buffer.data(), buffer.size(), ref );
        frameCounter++;
    }
    std::cout << "Read " << frameCounter << " frames\n";
}


int main( int argc, char *argv[] )
{
    if (argc < 5)
    {
        std::cerr << "Error: Insufficient arguments provided\n" <<
            "Usage: mjpgmuxer <output_filename> <width> <height> <framerate> [ frames ]\n";
        return 1;
    }

    std::string outfile = argv[1];
    int width     = atoi( argv[2] );
    int height    = atoi( argv[3] );
    float fps     = atof( argv[4] );
    int stream          = 0;
    std::string codecId = "V_MJPEG";

    MatroskaMuxer muxer;
    muxer.Set_OutputFilename( outfile );
    SetStreamHeader( muxer, stream, width, height, codecId );
    muxer.WriteHeaders();

    CopyFrames( muxer, stream, fps, argv + 5, argv + argc );

    muxer.CloseFile();

    return 0;
}

