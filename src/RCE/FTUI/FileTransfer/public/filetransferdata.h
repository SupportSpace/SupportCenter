#pragma once

#include <AidLib/Strings/tstring.h>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits/remove_pointer.hpp>

typedef boost::shared_ptr<boost::remove_pointer<HANDLE>::type> SPHandle;

/// always create file/folder
#define FT_CREATE_ALWAYS 0x001
/// directory is zipped
#define FT_ZIPPED_FOLDER 0x002

const unsigned int g_uTransferChunk = 65536;

enum EDriveTypes: DWORD
{
	DT_FLOPPY,
	DT_FIXED,
	DT_REMOVABLE,
	DT_CDROM,
	DT_REMOTE,
	DT_RAMDISK,
	DT_UNKNOW
};

enum FileType
{
	directory,	
	file
};
/// Describes file transfer status
enum TransferStatus
{
	starting,	/** starting operation */
	inprocess,	/** operation in progress*/
	completed   /** operation completed */
};
/// Describes files operation
enum TransferOpearation
{
	list_drives,		/** list drives opeartion */
	list_files,			/** list files opeartion */
	send_file,			/** send file opeartion */
	retrieve_file,		/** retrieve file opeartion */
	delete_file,		/** delete file opeartion */
	delete_directory,	/** delete directory opeartion */
	rename_file,		/** rename file opeartion */
	create_directory	/** create directory opeartion */
};

/// Server's response code
enum ResponseCode: DWORD
{
	operation_accepted		= 0,

	old_accept_operation,		/** opeartion was accepted */
	old_reject_operation,		/** opeartion was rejected */
	old_abort_operation,		/** opeartion was aborted */
	old_operation_completed,	/** opeartion completed */
	old_file_exist,			/** file exists */
	old_access_denided,		/** access is denided */
	old_unknown_message,		/** wrong protocol version */
	old_unknown_erorr,			/** wrong protocol version */

	operation_was_aborted	= 0xFFFFFFFE,
	operation_rejected		= 0xFFFFFFFF
};
/// Describes drive
struct DrivesInfo
{
	tstring			drive;		/** Drive's symbol (e.g. a:\  )  */
	EDriveTypes		type;		/** Drive's type */
	ULARGE_INTEGER	space;		/** Whole space of the disk */
	ULARGE_INTEGER	used_space;	/** used space in the disk*/
};

struct StatusInfo
{
	tstring fileName;		/** File's name which is realized operation  */
	tstring location;		/** File location */
	long	size;			/** File size */
	long    transferred;	/** Curent operation progress */
	TransferStatus status;	/** Operation status */
};

typedef std::vector<DrivesInfo> TDriveInfo;
typedef std::vector<WIN32_FIND_DATA> TFileInfo;


