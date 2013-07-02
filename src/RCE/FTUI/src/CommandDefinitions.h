//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CommandDefinitions.h
///
///  Declares commands that use by the UI and CCommandManager
///  
///  @author Alexander Novak @date 23.11.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////////////

enum EWidgetCommand
{
	cmd_NullCommand,
	cmd_Size,
	cmd_ButtonClick,
	cmd_BrowseForFiles,
	cmd_UpdateFileStatus,
	cmd_CreateDirectory,
	cmd_RenameFile,
	cmd_DeleteFile,
	cmd_CopyFile,
	cmd_MoveFile,
	cmd_ChangePanel,
	cmd_StartRenaming,
	cmd_EndRenaming,
	cmd_StartDirCreate,
	cmd_EndDirCreate,
	cmd_FileSelected,
	cmd_DragDropCopy,
	cmd_BeginDragDrop,
	cmd_BrowseForFilesUp,
	cmd_FilterChanged,
	cmd_DefaultTextAppeared
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
