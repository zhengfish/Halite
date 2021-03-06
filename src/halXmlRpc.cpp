
//         Copyright E�in O'Callaghan 2006 - 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"

#include <string>

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/xtime.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/tss.hpp>
#include <boost/format.hpp>
#include <boost/bind.hpp>

#include <http/xmlrpc/server.hpp>

#include "halXmlRpc.hpp"
//#include "halTorrent.hpp"

using namespace std;
using namespace boost;
using namespace http;
using namespace http::xmlrpc;

namespace hal
{

	XmlRpc& xmlRpc()
	{
		static XmlRpc x;
		return x;
	}
	
	class XmlRpc::XmlRpc_impl
	{
		friend class XmlRpc;
		
	private:
		XmlRpc_impl() :
			hostRunning(false)
		{}
		
		void host_procedure()
		{
			try
			{		
			hostRunning = true;
			theHost.run();
			hostRunning = false;		
			}
			catch(const exception&)
			{
			
			}	
		}
		
		host theHost;
		bool hostRunning;
		scoped_ptr<thread> hostThread;
	};
	
	XmlRpc::XmlRpc() :
		pimpl(new XmlRpc::XmlRpc_impl())
	{}
	
	void XmlRpc::bindHost(short port)
	{
		if (!pimpl->hostRunning)
		{
			pimpl->theHost.bind_to(port);
			pimpl->theHost.run();
		}
		else
		{
			rebindHost(port);
		}
	}
	
	void XmlRpc::rebindHost(short port)
	{
		pimpl->theHost.unbind();
		pimpl->theHost.bind_to(port);
	}
	
	void XmlRpc::stopHost()
	{
		pimpl->theHost.stop();
	}
	
//	static http::host host(80, 443);
//	static shared_ptr<thread> host_thread;
	
/*	class GetTorrentDetail_vec : public XmlRpcServerMethod
	{
	public:
		GetTorrentDetail_vec(XmlRpcServer* s) : 
			XmlRpcServerMethod("getTorrentDetail_vec", s) 
		{}
	
		void execute(XmlRpcValue& params, XmlRpcValue& result)
		{
			int nArgs = params.size();
			
			hal::torrentBriefDetails tbd = hal::getTorrents();
			if (tbd) 
			{				
				for(size_t i=0; i<tbd->size(); i++) 
				{
					wstring details = (wformat(L"%1$.2f%%, (D-U) %2$.2f-%3$.2f kb/s")
						% ((*tbd)[i].completion * 100)
						% ((*tbd)[i].speed.first/1024)
						% ((*tbd)[i].speed.second/1024)
						).str();
						
					result[i][0] = hal::wcstombs((*tbd)[i].filename);
					result[i][1] = hal::wcstombs(details);
				}
			}
		}
	
		std::string help() { return std::string("Get Torrent Details"); }
	
	} getTorrentDetail_vec(&Server);
	
	class GetPausedTorrents : public XmlRpcServerMethod
	{
	public:
		GetPausedTorrents(XmlRpcServer* s) : 
			XmlRpcServerMethod("getPausedTorrents", s) 
		{}
	
		void execute(XmlRpcValue& params, XmlRpcValue& result)
		{
			int nArgs = params.size();
			
			hal::torrentBriefDetails tbd = hal::getTorrents();
			if (tbd) 
			{		
				size_t j = 0;				
				for(size_t i=0; i<tbd->size(); i++) 
				{
					if ((*tbd)[i].status == L"Paused")					
						result[j++] = hal::wcstombs((*tbd)[i].filename);
				}
			}
		}
	
		std::string help() { return std::string("Get Paused Torrents"); }
	
	} getPausedTorrents(&Server);
	
	class GetActiveTorrents : public XmlRpcServerMethod
	{
	public:
		GetActiveTorrents(XmlRpcServer* s) : 
			XmlRpcServerMethod("getActiveTorrents", s) 
		{}
	
		void execute(XmlRpcValue& params, XmlRpcValue& result)
		{
			int nArgs = params.size();
			
			hal::torrentBriefDetails tbd = hal::getTorrents();
			if (tbd) 
			{		
				size_t j = 0;				
				for(size_t i=0; i<tbd->size(); i++) 
				{
					if ((*tbd)[i].status != L"Paused")					
						result[j++] = hal::wcstombs((*tbd)[i].filename);
				}
			}
		}
	
		std::string help() { return std::string("Get Active Torrents"); }
	
	} getActiveTorrents(&Server);
	
	class ResumeTorrent : public XmlRpcServerMethod
	{
	public:
		ResumeTorrent(XmlRpcServer* s) : 
			XmlRpcServerMethod("resumeTorrent", s) 
		{}
	
		void execute(XmlRpcValue& params, XmlRpcValue& result)
		{
			int nArgs = params.size();
			if (nArgs >= 2)
			{
				string filename = params[1];
				hal::resumeTorrent(hal::mbstowcs(filename));
			}			
		}
	
		std::string help() { return std::string("Resume Torrent"); }
	
	} resumeTorrent(&Server);
	
	class PauseTorrent : public XmlRpcServerMethod
	{
	public:
		PauseTorrent(XmlRpcServer* s) : 
			XmlRpcServerMethod("pauseTorrent", s) 
		{}
	
		void execute(XmlRpcValue& params, XmlRpcValue& result)
		{
			int nArgs = params.size();
			if (nArgs >= 2)
			{
				string filename = params[1];
				hal::pauseTorrent(hal::mbstowcs(filename));
			}			
		}
	
		std::string help() { return std::string("Pause Torrent"); }
	
	} pauseTorrent(&Server);
	
	class Hello : public XmlRpcServerMethod
	{
	public:
		Hello(XmlRpcServer* s) : XmlRpcServerMethod("Hello", s) 
		{}
	
		void execute(XmlRpcValue& params, XmlRpcValue& result)
		{
			int nArgs = params.size();
//			Log(wformat(L"%1%\r\n") % nArgs); 
			
			for (int i=0; i<nArgs; ++i)
			{
				switch (params[i].getType())
				{
					case XmlRpcValue::TypeInt:			
//						Log(wformat(L" %1%) Int:%2%\r\n") % i % int(params[i]));
					break;
					
					case XmlRpcValue::TypeString:			
//						Log(wformat(L" %1%) String\r\n") % i);
					break;
					
					case XmlRpcValue::TypeArray:			
//						Log(wformat(L" %1%) Array\r\n") % i);
					break;
					
				}
			}
			
			string resultstring = "Wayhaa";
			result[0] = resultstring;
			result[1] = 123;
			result[2][0] = 123;
			result[2][1] = "Hi";
		}
	
		std::string help() { return std::string("Say hello"); }
	
	} hello(&Server);
	
	class Close : public XmlRpcServerMethod
	{
	public:
		Close(XmlRpcServer* s) : XmlRpcServerMethod("close", s) 
		{}
	
		void execute(XmlRpcValue& params, XmlRpcValue& result)
		{}
	
		std::string help() { return std::string("Close Halite"); }
	
	} close(&Server);
	
	void RunServer()
	{
//		Log(wformat(L"Running Server.\r\n"));
		
		Server.bindAndListen(Port);
		
		// Enable introspection
		Server.enableIntrospection(true);
		
		// Wait for requests indefinitely
		Server.work(-1.0);
	  
//		Log(wformat(L"Shutting down server.\r\n"));	
	}
*/

	void host_procedure()
	{
		try
		{
		
		cout << "Running server...\r\n";
//		host.run();
	
		cout << "Finished\r\n";
		
		}
		catch(const exception&)
		{
//		cout << format("Thread Error: %1%.\r\n") % e.what();
		}	
	}
	
	bool initServer(int port)
	{
		try
		{
		
/*		http::xmlrpc::procedure_manager& man = http::server::add_xmlrpc_handler(host, "/xmlrpc");
		man.add_procedure("RemoteMethod", &remote_method);
		
		cout << "Initializing Host thread.\r\n";	
		
		host_tread.reset(new thread(&host_procedure));
*/		
		}
		catch(const exception&)
		{
//		cout << format("Main Error: %1%.\r\n") % e.what();
		}		
		
		return true;
	}
	
	void exitServer()
	{
//		host_tread.reset();
	}
	
}
