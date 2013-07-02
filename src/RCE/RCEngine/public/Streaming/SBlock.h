/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  SBlock.h
///
///  Structure of a block for TimeStamped streams
///
///  @author "Archer Software" Sogin M. @date 04.10.2006
///
////////////////////////////////////////////////////////////////////////
#pragma once

/// Type of block
typedef enum _EBlockType
{
	DATA,		///Data block
	TIMESTAMP	///Timestamp block
} EBlockType;

#define BLOCK_HEAD_SIZE (sizeof(unsigned int)+sizeof(EBlockType))

#pragma pack(push, 1)

/// Structure of a block for TimeStamped streams
typedef struct _SBlock
{
	/// size of a block (size of buf + BLOCK_HEAD_SIZE)
	unsigned int size;
	/// type of block
	EBlockType type;
	/// block data
	char buf[1];
} SBlock;

#pragma pack(pop)
