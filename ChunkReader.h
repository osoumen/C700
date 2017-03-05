//
//  ChunkReader.h
//  C700
//
//  Created by osoumen on 2017/01/24.
//
//

#ifndef __C700__ChunkReader__
#define __C700__ChunkReader__

#include "DataBuffer.h"
#include "FileAccess.h"

class ChunkReader : public FileAccess, public DataBuffer {
public:
	ChunkReader( int allocMemSize );
	ChunkReader( const void *data, int dataSize, bool copy=false );
    ChunkReader( const char *path );
	virtual ~ChunkReader();
	
	bool	addChunk( int type, const void* data, int byte );	//容量不足で全部を書き込めないときはfalse
    bool    addChunkWithHeader(int type, const unsigned char *data, int byte, unsigned char *header, int headerByte);
	bool	readChunkHead( int *type, long *byte );
	virtual bool    Write();
    
	typedef struct {
		int		type;
		int     size;	//ヘッダを除いたデータサイズ
	} MyChunkHead;
	
private:

};


#endif /* defined(__C700__ChunkReader__) */
