
//         Copyright Eóin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#define HAL_MAIN_LISTVIEW_BEGIN	 		30000
#define ID_LVM_DOWNLOAD_FOLDER          HAL_MAIN_LISTVIEW_BEGIN + 1
#define ID_LVM_RECHECK					HAL_MAIN_LISTVIEW_BEGIN + 2
#define ID_LVM_EDIT_FOLDERS				HAL_MAIN_LISTVIEW_BEGIN + 3
#define HAL_LISTVIEW_CONFIRMDELETE		HAL_MAIN_LISTVIEW_BEGIN + 4
#define HAL_LVM_QUEUE_VIEW				HAL_MAIN_LISTVIEW_BEGIN + 5
#define HAL_AUTO_MANAGED				HAL_MAIN_LISTVIEW_BEGIN + 6
#define HAL_UNMANAGED					HAL_MAIN_LISTVIEW_BEGIN + 7
#define HAL_MANAGED_DOWNLOADING			HAL_MAIN_LISTVIEW_BEGIN + 8
#define HAL_MANAGED_SEEDING				HAL_MAIN_LISTVIEW_BEGIN + 9
#define HAL_QUEUE_MOVE_TOP				HAL_MAIN_LISTVIEW_BEGIN + 10
#define HAL_QUEUE_MOVE_UP				HAL_MAIN_LISTVIEW_BEGIN + 11
#define HAL_QUEUE_MOVE_DOWN				HAL_MAIN_LISTVIEW_BEGIN + 12
#define HAL_QUEUE_MOVE_BOTTOM			HAL_MAIN_LISTVIEW_BEGIN + 13

#ifndef RC_INVOKED

#include "Halite.hpp"

#include <boost/array.hpp>
#include <boost/signals.hpp>

#include "global/string_conv.hpp"

#include "halIni.hpp"
#include "HaliteListViewCtrl.hpp"
#include "HaliteSortListViewCtrl.hpp"

class HaliteWindow;

class HaliteListViewCtrl :
	public CHaliteSortListViewCtrl<HaliteListViewCtrl, const hal::torrent_details_ptr>,
	public WTL::CCustomDraw<HaliteListViewCtrl>,
	private hal::IniBase<HaliteListViewCtrl>,
	private boost::noncopyable
{
protected:
	typedef HaliteListViewCtrl thisClass;
	typedef hal::IniBase<thisClass> iniClass;
	typedef CHaliteSortListViewCtrl<thisClass, const hal::torrent_details_ptr> listClass;
	typedef WTL::CCustomDraw<thisClass> ownDrawClass;
	typedef const hal::torrent_details_ptr tD;

	friend class listClass;
	
	struct ColumnAdapters
	{
	
	typedef listClass::ColumnAdapter ColAdapter_t;
	
	struct Name : public ColAdapter_t
	{	
		virtual int compare(tD& l, tD& r) { return hal::compare(l->name(), r->name()); }		
		virtual std::wstring print(tD& t) { return t->name(); }		
	};
	
/*	struct Filename : public ColAdapter_t
	{	
		virtual int compare(tD& l, tD& r) { return hal::compare(l->filename(), r->filename()); }		
		virtual std::wstring print(tD& t) { return t->filename(); }		
	};
*/	
	struct State : public ColAdapter_t
	{	
		virtual int compare(tD& l, tD& r) { return hal::compare(l->state(), r->state()); }		
		virtual std::wstring print(tD& t) { return t->state(); }		
	};
	
	struct Tracker : public ColAdapter_t
	{
		virtual int compare(tD& l, tD& r) { return hal::compare(l->currentTracker(), r->currentTracker()); }		
		virtual std::wstring print(tD& t) { return t->currentTracker(); }		
	};
	
	struct SpeedDown : public ColAdapter_t
	{
		virtual int compare(tD& l, tD& r) { return hal::compare(l->speed().first, r->speed().first); }		
		virtual std::wstring print(tD& t) 
		{
			return (hal::wform(L"%1$.2fkb/s") % (t->speed().first/1024)).str(); 
		}		
	};
	
	struct SpeedUp : public ColAdapter_t
	{
		virtual int compare(tD& l, tD& r) { return hal::compare(l->speed().second, r->speed().second); }		
		virtual std::wstring print(tD& t) 
		{
			return (hal::wform(L"%1$.2fkb/s") % (t->speed().second/1024)).str(); 
		}		
	};

	struct Progress : public ColAdapter_t
	{
		virtual int compare(tD& l, tD& r) { return hal::compare(l->completion(), r->completion()); }		
		virtual std::wstring print(tD& t) 
		{
			return (hal::wform(L"%1$.2f%%") % (t->completion()*100)).str(); 
		}		
	};

	struct Peers : public ColAdapter_t
	{
		virtual int compare(tD& l, tD& r) { return hal::compare(l->peers(), r->peers()); }		
		virtual std::wstring print(tD& t) 
		{
			return (hal::wform(L"%1% (%2%)") % t->peersConnected() % t->peers()).str(); 
		}
	};
	
	struct Seeds : public ColAdapter_t
	{
		virtual int compare(tD& l, tD& r) { return hal::compare(l->seeds(), r->seeds()); }				
		virtual std::wstring print(tD& t) 
		{
			return (hal::wform(L"%1% (%2%)") % t->seedsConnected() % t->seeds()).str(); 
		}	
	};
	
	struct ETA : public ColAdapter_t
	{
		virtual int compare(tD& l, tD& r) { return hal::compare(l->estimatedTimeLeft(), r->estimatedTimeLeft()); }		
		virtual std::wstring print(tD& t) 
		{ 
			if (!t->estimatedTimeLeft().is_special())
			{
				return hal::from_utf8(
					boost::posix_time::to_simple_string(t->estimatedTimeLeft()));
			}
			else
			{
				return hal::app().res_wstr(HAL_INF);		
			}
		}		
	};
	
	struct UpdateTrackerIn : public ColAdapter_t
	{
		virtual int compare(tD& l, tD& r) { return hal::compare(l->updateTrackerIn(), r->updateTrackerIn()); }		
		virtual std::wstring print(tD& t) 
		{ 
			if (!t->updateTrackerIn().is_special())
			{
				return hal::from_utf8(
					boost::posix_time::to_simple_string(t->updateTrackerIn()));
			}
			else
			{
				return hal::app().res_wstr(HAL_INF);		
			}
		}		
	};
	
	struct Ratio : public ColAdapter_t
	{
		virtual int compare(tD& l, tD& r)	
		{ 
			float left = (l->totalPayloadDownloaded()) 
					? static_cast<float>(l->totalPayloadUploaded())
						/ static_cast<float>(l->totalPayloadDownloaded())
					: 0;
			
			float right = (r->totalPayloadDownloaded()) 
					? static_cast<float>(r->totalPayloadUploaded())
						/ static_cast<float>(r->totalPayloadDownloaded())
					: 0;
			
			return hal::compare(left, right); 
		}		
		virtual std::wstring print(tD& t)
		{
			float ratio = (t->totalPayloadDownloaded()) 
					? static_cast<float>(t->totalPayloadUploaded())
						/ static_cast<float>(t->totalPayloadDownloaded())
					: 0;
			
			return (hal::wform(L"%1$.2f") % ratio).str(); 
		}		
	};
	
	struct DistributedCopies : public ColAdapter_t
	{
		virtual int compare(tD& l, tD& r) { return hal::compare(l->distributedCopies(), r->distributedCopies()); }		
		virtual std::wstring print(tD& t) 
		{ 
			float copies = t->distributedCopies();
			
			if (copies < 0)
				return L"Seeding"; 
			else
				return (hal::wform(L"%1$.2f") % copies).str();		
		}		
	};

	struct Remaining : public ColAdapter_t
	{
		virtual int compare(tD& l, tD& r)
		{
			boost::int64_t left = l->totalWanted()-l->totalWantedDone();
			boost::int64_t right = r->totalWanted()-r->totalWantedDone();
			
			return hal::compare(left, right); 
		}
		
		virtual std::wstring print(tD& t) 
		{
			return (hal::wform(L"%1$.2fMB") % (static_cast<float>(t->totalWanted()-t->totalWantedDone())/(1024*1024))).str(); 
		}		
	};

	struct Completed : public ColAdapter_t
	{
		virtual int compare(tD& l, tD& r)
		{			
			return hal::compare(l->totalWantedDone(), r->totalWantedDone()); 
		}
		
		virtual std::wstring print(tD& t) 
		{
			return (hal::wform(L"%1$.2fMB") % (static_cast<float>(t->totalWantedDone())/(1024*1024))).str(); 
		}		
	};

	struct TotalWanted : public ColAdapter_t
	{
		virtual int compare(tD& l, tD& r)
		{		
			return hal::compare(l->totalWanted(), r->totalWanted()); 
		}
		
		virtual std::wstring print(tD& t) 
		{
			return (hal::wform(L"%1$.2fMB") % (static_cast<float>(t->totalWanted())/(1024*1024))).str(); 
		}		
	};

	struct Downloaded : public ColAdapter_t
	{
		virtual int compare(tD& l, tD& r)
		{		
			return hal::compare(l->totalPayloadDownloaded(), r->totalPayloadDownloaded()); 
		}
		
		virtual std::wstring print(tD& t) 
		{
			return (hal::wform(L"%1$.2fMB") % (static_cast<float>(t->totalPayloadDownloaded())/(1024*1024))).str(); 
		}		
	};

	struct Uploaded : public ColAdapter_t
	{
		virtual int compare(tD& l, tD& r)
		{		
			return hal::compare(l->totalPayloadUploaded(), r->totalPayloadUploaded()); 
		}
		
		virtual std::wstring print(tD& t) 
		{
			return (hal::wform(L"%1$.2fMB") % (static_cast<float>(t->totalPayloadUploaded())/(1024*1024))).str(); 
		}		
	};

	struct ActiveTime : public ColAdapter_t
	{
		virtual int compare(tD& l, tD& r)	{ return hal::compare(l->active(), r->active()); }		
		virtual std::wstring print(tD& t) 
		{ 
			if (!t->active().is_special())
			{
				return hal::from_utf8(
					boost::posix_time::to_simple_string(t->active()));
			}
			else
			{
				return hal::app().res_wstr(HAL_INF);		
			}
		}		
	};
	
	struct SeedingTime : public ColAdapter_t
	{
		virtual int compare(tD& l, tD& r) { return hal::compare(l->seeding(), r->seeding()); }		
		virtual std::wstring print(tD& t) 
		{ 
			if (!t->seeding().is_special())
			{
				return hal::from_utf8(
					boost::posix_time::to_simple_string(t->seeding()));
			}
			else
			{
				return hal::app().res_wstr(HAL_INF);		
			}
		}		
	};
	
	struct StartTime : public ColAdapter_t
	{
		virtual int compare(tD& l, tD& r)	{ return hal::compare(l->startTime(), r->startTime()); }		
		virtual std::wstring print(tD& t) 
		{ 
			if (!t->startTime().is_special())
			{
				return hal::from_utf8(
					boost::posix_time::to_simple_string(t->startTime()));
			}
			else
			{
				return hal::app().res_wstr(IDS_NA);		
			}
		}		
	};
	
	struct FinishTime : public ColAdapter_t
	{
		virtual int compare(tD& l, tD& r) { return hal::compare(l->finishTime(), r->finishTime()); }		
		virtual std::wstring print(tD& t) 
		{ 
			if (!t->finishTime().is_special())
			{
				return hal::from_utf8(
					boost::posix_time::to_simple_string(t->finishTime()));
			}
			else
			{
				return hal::app().res_wstr(IDS_NA);		
			}
		}		
	};
	
	struct Managed : public ColAdapter_t
	{
		virtual int compare(tD& l, tD& r) { return hal::compare(l->managed(), r->managed()); }		
		virtual std::wstring print(tD& t) 
		{ 
			if (t->managed())
			{
				return L"Yes";; 
			}
			else
			{
				return L"No";; 	
			}
		}		
	};
	
	struct QueuePosition : public ColAdapter_t
	{
		virtual int compare(tD& l, tD& r) { return hal::compare(l->queue_position(), r->queue_position()); }		
		virtual std::wstring print(tD& t) 
		{ 
			if (t->queue_position() != -1)
			{
				return (hal::wform(L"%1%") % (t->queue_position())).str(); 
			}
			else
			{
				return hal::app().res_wstr(IDS_NA);		
			}
		}		
	};
	
	};

public:
	enum { 
		LISTVIEW_ID_MENU = HAL_LISTVIEW_MENU,
		LISTVIEW_ID_COLUMNNAMES = HAL_LISTVIEW_COLUMNS,
		LISTVIEW_ID_COLUMNWIDTHS = HAL_LISTVIEW_DEFAULTS
	};

	HaliteListViewCtrl(HaliteWindow& HalWindow);

	BEGIN_MSG_MAP_EX(HaliteListViewCtrl)
		MSG_WM_SHOWWINDOW(OnShowWindow)
		MSG_WM_DESTROY(OnDestroy)

		COMMAND_ID_HANDLER(ID__LVM_PAUSE, OnPause)
		COMMAND_ID_HANDLER(ID_LVM_STOP, OnStop)
		COMMAND_ID_HANDLER(ID_LVM_RESUME, OnResume)
		COMMAND_ID_HANDLER(ID_LVM_RECHECK, OnRecheck)
		COMMAND_ID_HANDLER(ID_LVM_REMOVE_T, OnRemove)
		COMMAND_ID_HANDLER(ID_LVM_REMOVE_TD, OnRemoveWipeFiles)
		COMMAND_ID_HANDLER(ID_LVM_DOWNLOAD_FOLDER, OnDownloadFolder)
		COMMAND_ID_HANDLER(ID_LVM_EDIT_FOLDERS, OnEditFolders)

		COMMAND_ID_HANDLER(HAL_LVM_QUEUE_VIEW, OnQueueView)

        CHAIN_MSG_MAP_ALT(ownDrawClass, 1)
		CHAIN_MSG_MAP(listClass)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	void OnShowWindow(UINT, INT);
	void OnDestroy();
	void saveSettings();
	void uiUpdate(const hal::torrent_details_manager& allTorrents); 

	DWORD OnPrePaint(int idCtrl, LPNMCUSTOMDRAW lpNMCD);
	DWORD OnItemPrePaint(int idCtrl, LPNMCUSTOMDRAW lpNMCD);

	LRESULT OnPause(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnResume(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRecheck(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRemoveFocused(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRemoveWipeFiles(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnDownloadFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnEditFolders(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnQueueView(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	void queue_view_mode();

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
		using boost::serialization::make_nvp;

		if (version > 2)
			ar & boost::serialization::make_nvp("listview", 
				boost::serialization::base_object<listClass>(*this));
		
		ar & make_nvp("queue_view", queue_view_);
    }
	
	tD CustomItemConversion(LVCompareParam* param, int iSortCol);

	int CompareItemsCustom(LVCompareParam* pItem1, LVCompareParam* pItem2, int iSortCol)
	{
		hal::mutex_update_lock<listClass> lock(*this);
		
		tD left = CustomItemConversion(pItem1, iSortCol);
		tD right = CustomItemConversion(pItem2, iSortCol);
		
		return CustomItemComparision(left, right, iSortCol);
	}

private:
	void OnAttach();
	void OnDetach();
	
	enum { NumberOfColumns_s = 23 };
	
	HaliteWindow& halWindow_;
	bool queue_view_;
};

BOOST_CLASS_VERSION(HaliteListViewCtrl, 3)
typedef HaliteListViewCtrl::SelectionManager ListViewManager;

#endif // RC_INVOKED
