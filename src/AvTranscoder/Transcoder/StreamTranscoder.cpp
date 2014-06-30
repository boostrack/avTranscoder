
#include "StreamTranscoder.hpp"

#include <AvTranscoder/CodedStream/AvInputStream.hpp>

namespace avtranscoder
{

StreamTranscoder::StreamTranscoder(
		InputStream& inputStream,
		OutputStream& outputStream,
		OutputFile& outputFile
	)
	: _inputStream( &inputStream )
	, _outputStream( &outputStream )
	, _frameBuffer( NULL )
	, _videoFrameBuffer( NULL )
	, _audioFrameBuffer( NULL )
	, _inputEssence( NULL )
	, _outputEssence( NULL )
	, _transcodeStream( false )
{
	// create a re-wrapping case
	switch( _inputStream->getStreamType() )
	{
		case AVMEDIA_TYPE_VIDEO :
		{
			outputFile.addVideoStream( _inputStream->getVideoDesc() );
			break;
		}
		case AVMEDIA_TYPE_AUDIO :
		{
			outputFile.addAudioStream( _inputStream->getAudioDesc() );
			break;
		}
		default:
			break;
	}
}

StreamTranscoder::StreamTranscoder(
		InputStream& inputStream,
		OutputStream& outputStream,
		OutputFile& outputFile,
		Profile::ProfileDesc& profile
	)
	: _inputStream( &inputStream )
	, _outputStream( &outputStream )
	, _frameBuffer( NULL )
	, _videoFrameBuffer( NULL )
	, _audioFrameBuffer( NULL )
	, _inputEssence( NULL )
	, _outputEssence( NULL )
	, _transcodeStream( true )
{
	// create a transcode case
	switch( _inputStream->getStreamType() )
	{
		case AVMEDIA_TYPE_VIDEO :
		{
			_inputEssence = new InputVideo( *static_cast<AvInputStream*>( _inputStream ) );
			_inputEssence->setup();

			OutputVideo* outputVideo = new OutputVideo();
			_outputEssence = outputVideo;

			_outputEssence->setProfile( profile );
			outputFile.addVideoStream( outputVideo->getVideoDesc() );
			_videoFrameBuffer = new Image( outputVideo->getVideoDesc().getImageDesc() );
			_frameBuffer = _videoFrameBuffer;
			
			break;
		}
		case AVMEDIA_TYPE_AUDIO :
		{
			_inputEssence = new InputAudio( *static_cast<AvInputStream*>( _inputStream ) );
			_inputEssence->setup();
			
			OutputAudio* outputAudio = new OutputAudio();
			_outputEssence = outputAudio;

			_outputEssence->setProfile( profile );
			outputFile.addAudioStream( outputAudio->getAudioDesc() );
			_audioFrameBuffer = new AudioFrame( outputAudio->getAudioDesc().getFrameDesc() );
			_frameBuffer = _audioFrameBuffer;
			
			break;
		}
		default:
			break;
	}
}

StreamTranscoder::StreamTranscoder(
		InputEssence& inputEssence,
		OutputStream& outputStream,
		OutputFile& outputFile,
		Profile::ProfileDesc& profile
	)
	: _inputStream( NULL )
	, _outputStream( &outputStream )
	, _frameBuffer( NULL )
	, _videoFrameBuffer( NULL )
	, _audioFrameBuffer( NULL )
	, _inputEssence( &inputEssence )
	, _outputEssence( NULL )
	, _transcodeStream( true )
{
	// create an encoder case from a dummy
	switch( _inputStream->getStreamType() )
	{
		case AVMEDIA_TYPE_VIDEO :
		{
			OutputVideo* outputVideo = new OutputVideo();
			_outputEssence = outputVideo;

			_outputEssence->setProfile( profile );
			outputFile.addVideoStream( outputVideo->getVideoDesc() );
			_videoFrameBuffer = new Image( outputVideo->getVideoDesc().getImageDesc() );
			_frameBuffer = _videoFrameBuffer;
			
			break;
		}
		case AVMEDIA_TYPE_AUDIO :
		{
			OutputAudio* outputAudio = new OutputAudio();
			_outputEssence = outputAudio;

			_outputEssence->setProfile( profile );
			outputFile.addAudioStream( outputAudio->getAudioDesc() );
			_audioFrameBuffer = new AudioFrame( outputAudio->getAudioDesc().getFrameDesc() );
			_frameBuffer = _audioFrameBuffer;
			
			break;
		}
		default:
			break;
	}
}

StreamTranscoder::~StreamTranscoder()
{
	if( _frameBuffer )
		delete _frameBuffer;
	if( _inputEssence )
		delete _inputEssence;
	if( _outputEssence )
		delete _outputEssence;
}


bool StreamTranscoder::processFrame()
{
	DataStream dataStream;
	if( ! _transcodeStream )
	{
		if( ! _inputStream->readNextPacket( dataStream ) )
			return false;
		_outputStream->wrap( dataStream );
		return true;
	}

	if( _inputEssence->readNextFrame( *_frameBuffer ) )
	{
		_outputEssence->encodeFrame( *_frameBuffer, dataStream );
	}
	else if( ! _outputEssence->encodeFrame( dataStream ) )
	{
		return false;
	}

	_outputStream->wrap( dataStream );
	return true;
}

}