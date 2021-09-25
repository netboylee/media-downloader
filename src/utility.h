/*
 *
 *  Copyright (c) 2021
 *  name : Francis Banyikwa
 *  email: mhogomchungu@gmail.com
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UTILITY_H
#define UTILITY_H

#include <QStringList>
#include <QString>
#include <QProcess>
#include <QMenu>
#include <QPushButton>
#include <QTimer>
#include <QThread>

#include <type_traits>
#include <memory>
#include <iostream>

#include "translator.h"

#include "ui_mainwindow.h"

#include "engines.h"

#include "tableWidget.h"

#include "util.hpp"

class Context ;

class tabManager ;

namespace Ui
{
	class MainWindow ;
}

namespace utility
{
	struct Debug
	{
		template< typename T >
		Debug& operator<<( const T& e )
		{
			std::cout << e << std::endl ;
			return *this ;
		}
		Debug& operator<<( const QString& e )
		{
			std::cout << e.toStdString() << std::endl ;
			return *this ;
		}
		Debug& operator<<( const QByteArray& e )
		{
			std::cout << e.data() << std::endl ;
			return *this ;
		}
		Debug& operator<<( const QStringList& e )
		{
			for( const auto& s : e ){

				std::cout<< s.toStdString() << std::endl ;
			}
			std::cout << std::endl ;
			return *this ;
		}
	};

	extern Debug debug ;

	class args
	{
	public:
		args( const QString& e )
		{
			if( !e.isEmpty() ){

				m_otherOptions = util::splitPreserveQuotes( e ) ;

				if( !m_otherOptions.isEmpty() ){

					m_quality = m_otherOptions.takeFirst() ;
				}
			}
		}
		const QString& quality() const
		{
			return m_quality ;
		}
		const QStringList& otherOptions() const
		{
			return m_otherOptions ;
		}
	private:
		QString m_quality ;
		QStringList m_otherOptions ;
	} ;

	template< typename T >
	void removeArgument( QStringList& s,const T& e )
	{
		s.removeAll( e ) ;
	}

	template< typename T >
	void removeArgumentWithOption( QStringList& s,const T& e )
	{
		for( int i = 0 ; i < s.size() ; i++ ){

			if( s[ i ] == e ){

				if( i + 1 < s.size() ){

					s.removeAt( i + 1 ) ;
				}

				s.removeAt( i ) ;

				break ;
			}
		}
	}

	QString failedToFindExecutableString( const QString& cmd ) ;
	int concurrentID() ;
	void saveDownloadList( const Context&,QMenu&,tableWidget& ) ;
	void wait( int time ) ;
	void waitForOneSecond() ;
	void openDownloadFolderPath( const QString& ) ;
	QString homePath() ;
	QString python3Path() ;
	QString clipboardText() ;
	bool platformIsWindows() ;
	bool platformIs32BitWindows() ;
	bool platformIsLinux() ;
	bool platformIsOSX() ;
	bool platformIsNOTWindows() ;
	bool isRelativePath( const QString& ) ;
	QString downloadFolder( const Context& ctx ) ;
	const QProcessEnvironment& processEnvironment( const Context& ctx ) ;

	QStringList updateOptions( const engines::engine& engine,
				   settings&,
				   const utility::args& args,
				   const QString& indexAsString,
				   const QStringList& urls ) ;

	bool hasDigitsOnly( const QString& e ) ;

	class contextState
	{
	public:
		contextState() :
			m_noneAreRunning( true ),
			m_finishedSuccess( false )
		{
		}
		contextState( bool r ) :
			m_noneAreRunning( r ),
			m_finishedSuccess( false )
		{
		}
		contextState( bool r,bool f ) :
			m_noneAreRunning( r ),
			m_finishedSuccess( f )
		{
		}
		bool noneAreRunning() const
		{
			return m_noneAreRunning ;
		}
		bool finishedSuccess() const
		{
			return m_finishedSuccess ;
		}
		bool showLogWindow() const
		{
			return m_showLogWindow ;
		}
		void setShowLogWindow()
		{
			m_showLogWindow = true ;
		}
		bool clear() const
		{
			return m_clear ;
		}
		void setClear()
		{
			m_clear = true ;
		}
	private:
		bool m_noneAreRunning ;
		bool m_finishedSuccess ;
		bool m_showLogWindow = false ;
		bool m_clear = false ;
	};

	enum class PlayListButtonName{ DownloadRange,PlaylistUrl,None } ;
	template< typename Settings,typename TabName >
	inline bool showHistory( QLineEdit& lineEdit,
				 const QStringList& history,
				 Settings& settings,
				 TabName tabName,
				 PlayListButtonName pbn = PlayListButtonName::None )
	{
		if( history.isEmpty() ){

			return false ;
		}else{
			bool s = false ;

			QMenu m ;

			QObject::connect( &m,&QMenu::triggered,[ & ]( QAction * ac ){

				auto m = ac->objectName() ;

				if( m == "Clear" ){

					if( pbn == utility::PlayListButtonName::None ){

						settings.clearOptionsHistory( tabName ) ;

					}else if( pbn == utility::PlayListButtonName::DownloadRange ){

						settings.clearPlaylistRangeHistory() ;
					}else{
						settings.clearPlaylistUrlHistory() ;
					}
				}else{
					s = true ;

					lineEdit.setText( ac->objectName() ) ;
				}
			} ) ;

			for( const auto& it : history ){

				auto ss = settings.stringTruncationSize() ;

				if( it.size() < ss ){

					m.addAction( it )->setObjectName( it ) ;
				}else{
					auto sss = ss / 2 ;

					auto a = it.mid( 0,sss ) ;
					auto b = it.mid( it.size() - sss ) ;
					auto ac = m.addAction( a + "..." + b ) ;
					ac->setObjectName( it ) ;
					ac->setToolTip( it ) ;
				}
			}

			m.addSeparator() ;

			m.addAction( QObject::tr( "Clear" ) )->setObjectName( "Clear" ) ;

			m.exec( QCursor::pos() ) ;

			return s ;
		}
	}

	template< typename Function >
	void appendContextMenu( QMenu& m,utility::contextState c,Function function )
	{
		auto ac = m.addAction( QObject::tr( "Show Log Window" ) ) ;

		QObject::connect( ac,&QAction::triggered,[ &function,&c ](){

			c.setShowLogWindow() ;

			function( c ) ;
		} ) ;

		ac = m.addAction( QObject::tr( "Clear" ) ) ;

		ac->setEnabled( c.noneAreRunning() ) ;

		QObject::connect( ac,&QAction::triggered,[ &function,&c ](){

			c.setClear() ;

			function( c ) ;
		} ) ;

		m.exec( QCursor::pos() ) ;
	}

	class selectedAction
	{
	public:
		static const char * CLEARSCREEN ;
		static const char * CLEAROPTIONS ;
		static const char * OPENFOLDER ;

		selectedAction( QAction * ac ) : m_ac( ac )
		{
		}
		bool clearOptions() const
		{
			return m_ac->objectName() == utility::selectedAction::CLEAROPTIONS ;
		}
		bool clearScreen() const
		{
			return m_ac->objectName() == utility::selectedAction::CLEARSCREEN ;
		}
		bool openFolderPath() const
		{
			return m_ac->objectName() == utility::selectedAction::OPENFOLDER ;
		}
		QString text() const
		{
			return m_ac->text() ;
		}
		QString objectName() const
		{
			return m_ac->objectName() ;
		}
	private:
		QAction * m_ac ;
	};

	QMenu * setUpMenu( const Context& ctx,
			   const QStringList&,
			   bool addClear,
			   bool addOpenFolder,
			   bool combineText,
			   QWidget * parent ) ;

	template< typename Function >
	void setMenuOptions( const Context& ctx,
			     const QStringList& opts,
			     bool addClear,
			     bool addOpenFolder,
			     QPushButton * w,
			     Function function )
	{
		auto m = w->menu() ;

		if( m ){

			m->deleteLater() ;
		}

		auto menu = utility::setUpMenu( ctx,opts,addClear,addOpenFolder,false,w ) ;

		w->setMenu( menu ) ;

		QObject::connect( menu,&QMenu::triggered,std::move( function ) ) ;
	}

	template< typename WhenDone,typename WithData >
	void run( const QString& cmd,const QStringList& args,WhenDone w,WithData p )
	{
		util::run( cmd,args,[]( QProcess& ){},std::move( w ),std::move( p ) ) ;
	}

	template< typename Function,typename FunctionConnect >
	class Conn
	{
	public:
		Conn( Function function,
		      FunctionConnect functionConnect ) :
			m_function( std::move( function ) ),
			m_functionConnect( std::move( functionConnect ) )
		{
		}
		template< typename Fnt >
		void connect( Fnt function )
		{
			auto fnt = [ this,function = std::move( function ) ]( int index ){

				function( m_function,index ) ;
			} ;

			m_conn = m_functionConnect( std::move( fnt ) ) ;
		}
		void disconnect()
		{
			QObject::disconnect( m_conn ) ;
		}
	private:
		Function m_function ;
		FunctionConnect m_functionConnect ;
		QMetaObject::Connection m_conn ;
	};

	template< typename Function,typename FunctionConnect >
	utility::Conn< Function,FunctionConnect > make_conn( Function f,FunctionConnect c )
	{
		return utility::Conn< Function,FunctionConnect >( std::move( f ),std::move( c ) ) ;
	}

	class Terminator : public QObject
	{
		Q_OBJECT
	public:
		static util::result< int > terminate( int argc,char ** argv ) ;

		template< typename Object,typename Member >
		auto setUp( Object obj,Member member,int idx )
		{
			return utility::make_conn( []( const engines::engine& engine,QProcess& exe,int index,int idx ){

				return utility::Terminator::terminate( engine,exe,index,idx ) ;

			},[ idx,obj,member,this ]( auto function ){

				Q_UNUSED( this ) //Older version of gcc seems to require capturing "this".

				return QObject::connect( obj,member,[ idx,function = std::move( function ) ](){

					function( idx ) ;
				} ) ;
			} ) ;
		}
		auto setUp()
		{
			return utility::make_conn( []( const engines::engine& engine,QProcess& exe,int index,int idx ){

				return utility::Terminator::terminate( engine,exe,index,idx ) ;

			},[ this ]( auto function ){

				using e = void( Terminator::* )( int ) ;

				auto m = static_cast< e >( &utility::Terminator::terminate ) ;

				return QObject::connect( this,m,[ function = std::move( function ) ]( int index ){

					function( index ) ;
				} ) ;
			} ) ;
		}
		void terminateAll( QTableWidget& t )
		{
			for( int i = 0 ; i < t.rowCount() ; i++ ){

				this->terminate( i ) ;
			}
		}
	signals :
		void terminate( int index ) ;
	private:
		static bool terminate( QProcess& ) ;
		static bool terminate( const engines::engine&,QProcess& exe,int index,int idx )
		{
			if( index == idx ){

				return utility::Terminator::terminate( exe ) ;
			}else{
				return false ;
			}
		}
	};

	class ProcessExitState
	{
	public:
		ProcessExitState( bool c,int s,int d,QProcess::ExitStatus e ) :
			m_cancelled( c ),
			m_exitCode( s ),
			m_duration( d ),
			m_exitStatus( e )
		{
		}
		int exitCode() const
		{
			return m_exitCode ;
		}
		QProcess::ExitStatus exitStatus() const
		{
			return m_exitStatus ;
		}
		bool cancelled() const
		{
			return m_cancelled ;
		}
		bool success() const
		{
			return m_exitCode == 0 && m_exitStatus == QProcess::ExitStatus::NormalExit ;
		}
		int duration() const
		{
			return m_duration ;
		}
	private:
		bool m_cancelled = false ;
		int m_exitCode ;
		int m_duration ;
		QProcess::ExitStatus m_exitStatus ;
	};

	class ProcessOutputChannels
	{
	public:
		ProcessOutputChannels() :
			m_channelMode( QProcess::ProcessChannelMode::MergedChannels )
		{
		}
		ProcessOutputChannels( QProcess::ProcessChannel c ) :
			m_channelMode( QProcess::ProcessChannelMode::SeparateChannels ),
			m_channel( c )
		{
		}
		QProcess::ProcessChannelMode channelMode() const
		{
			return m_channelMode ;
		}
		QProcess::ProcessChannel channel() const
		{
			return m_channel ;
		}
	private:
		QProcess::ProcessChannelMode m_channelMode ;
		QProcess::ProcessChannel m_channel ;
	} ;

	template< typename Tlogger,
		  typename Options,
		  typename Connection >
	class context
	{
	public:
		context( const engines::engine& engine,
			 QProcess& exe,
			 ProcessOutputChannels channels,
			 Tlogger logger,
			 Options options,
			 Connection conn ) :
			m_engine( engine ),
			m_logger( std::move( logger ) ),
			m_options( std::move( options ) ),
			m_conn( std::move( conn ) ),
			m_channels( channels ),
			m_cancelled( false )
		{
			if( m_engine.replaceOutputWithProgressReport() ){

				QObject::connect( &m_timer,&QTimer::timeout,[ this ]{

					m_logger.add( [ this ]( Logger::Data& e,int id ){

						m_engine.processData( e,m_timeCounter.stringElapsedTime(),id ) ;
					} ) ;
				} ) ;

				m_timer.start( 1000 ) ;
			}

			m_conn.connect( [ this,&exe ]( auto& function,int index ){

				auto m = function( m_engine,exe,m_options.index(),index ) ;

				if( m ){

					m_cancelled = true ;
				}

				return m ;
			} ) ;
		}
		void postData( const QByteArray& data )
		{
			m_timer.stop() ;

			if( !m_cancelled ){

				if( m_options.listRequested() ){

					m_data += data ;
				}

				m_logger.add( [ this,&data ]( Logger::Data& e,int id ){

					m_engine.processData( e,data,id ) ;
				} ) ;
			}
		}
		bool debug()
		{
			return m_options.debug() ;
		}
		void done( int s,QProcess::ExitStatus e )
		{
			m_conn.disconnect() ;

			m_timer.stop() ;

			if( m_options.listRequested() ){

				m_options.listRequested( util::split( m_data,'\n' ) ) ;
			}

			auto m = m_timeCounter.elapsedTime() ;
			m_options.done( ProcessExitState( m_cancelled,s,m,std::move( e ) ) ) ;
		}
		const ProcessOutputChannels& outputChannels()
		{
			return m_channels ;
		}
	private:
		const engines::engine& m_engine ;
		Tlogger m_logger ;
		Options m_options ;
		Connection m_conn ;
		ProcessOutputChannels m_channels ;
		QTimer m_timer ;
		engines::engine::functions::timer m_timeCounter ;
		QByteArray m_data ;
		bool m_cancelled ;
	} ;

	template< typename Connection,
		  typename Tlogger,
		  typename Options >
	void run( const engines::engine& engine,
		  const QStringList& args,
		  const QString& quality,
		  Options options,
		  Tlogger logger,
		  Connection conn,
		  ProcessOutputChannels channels = ProcessOutputChannels() )
	{
		engines::engine::exeArgs::cmd cmd( engine.exePath(),args,options.downloadFolder() ) ;

		const auto& exe = cmd.exe() ;

		if( !QFile::exists( exe ) ){

			logger.add( utility::failedToFindExecutableString( exe ) ) ;

			options.done( ProcessExitState( false,-1,-1,QProcess::ExitStatus::NormalExit ) ) ;

			return ;
		}

		options.disableAll() ;

		using ctx_t = utility::context< Tlogger,Options,Connection > ;

		using unique_ptr_ctx_t = std::unique_ptr< ctx_t > ;

		util::run( exe,cmd.args(),[ &,logger = std::move( logger ),options = std::move( options ) ]( QProcess& exe )mutable{

			exe.setProcessEnvironment( options.processEnvironment() ) ;

			logger.add( "cmd: " + engine.commandString( cmd ) ) ;

			const auto& df = options.downloadFolder() ;

			if( !QFile::exists( df ) ){

				QDir().mkpath( df ) ;
			}

			exe.setWorkingDirectory( df ) ;

			exe.setProcessChannelMode( channels.channelMode() ) ;

			return std::make_unique< ctx_t >( engine,
							  exe,
							  channels,
							  std::move( logger ),
							  std::move( options ),
							  std::move( conn ) ) ;

		},[ &engine,quality ]( QProcess& exe ){

			engine.sendCredentials( quality,exe ) ;

		},[]( int s,QProcess::ExitStatus e,unique_ptr_ctx_t& ctx ){

			ctx->done( s,std::move( e ) ) ;

		},[]( QProcess::ProcessChannel channel,QByteArray data,unique_ptr_ctx_t& ctx ){

			if( ctx->debug() ){

				utility::Debug() << data ;
				utility::Debug() << "------------------------" ;
			}

			const auto& channels = ctx->outputChannels() ;

			if( channels.channelMode() == QProcess::ProcessChannelMode::MergedChannels ){

				ctx->postData( std::move( data ) ) ;

			}else if( channels.channelMode() == QProcess::ProcessChannelMode::SeparateChannels ){

				auto c = channels.channel() ;

				if( c == QProcess::ProcessChannel::StandardOutput && channel == c ){

					ctx->postData( std::move( data ) ) ;

				}else if( c == QProcess::ProcessChannel::StandardError && channel == c ){

					ctx->postData( std::move( data ) ) ;
				}
			}
		} ) ;
	}

	template< typename List,
		  std::enable_if_t< std::is_lvalue_reference< List >::value,int > = 0 >
	class reverseIterator
	{
	public:
		typedef typename std::remove_reference_t< std::remove_cv_t< List > > ::value_type value_type ;
		typedef typename std::remove_reference_t< std::remove_cv_t< List > > ::size_type size_type ;

	        reverseIterator( List s ) :
		        m_list( s ),
			m_index( m_list.size() - 1 )
		{
		}
		bool hasNext() const
		{
			return m_index > -1 ;
		}
		void reset()
		{
			m_index = m_list.size() - 1 ;
		}
		auto& next()
		{
			auto s = static_cast< typename reverseIterator< List >::size_type >( m_index-- ) ;

			return m_list[ s ] ;
		}
		template< typename Function,
			  util::types::has_bool_return_type< Function,typename reverseIterator< List >::value_type > = 0 >
		void forEach( Function function )
		{
			while( this->hasNext() ){

				if( function( this->next() ) ){

					break ;
				}
			}
		}
		template< typename Function,
			  util::types::has_void_return_type< Function,typename reverseIterator< List >::value_type > = 0 >
		void forEach( Function function )
		{
			while( this->hasNext() ){

				function( this->next() ) ;
			}
		}
	private:
		List m_list ;
		int m_index ;
	} ;

	template< typename List >
	auto make_reverseIterator( List&& l )
	{
		return reverseIterator< decltype( l ) >( std::forward< List >( l ) ) ;
	}

	class MediaEntry
	{
	public:
		MediaEntry( const QString& url ) :
			m_url( url ),
			m_json( QByteArray() )
		{
		}
		MediaEntry( const QString& uiText,const QString& url ) :
			m_title( uiText ),
			m_url( url ),
			m_json( QByteArray() )
		{
		}
		MediaEntry( const QByteArray& data ) : m_json( data )
		{
			if( m_json ){

				auto object = m_json.doc().object() ;

				m_title        = object.value( "title" ).toString() ;
				m_thumbnailUrl = object.value( "thumbnail" ).toString() ;
				m_url          = object.value( "webpage_url" ).toString() ;
				m_uploadDate   = object.value( "upload_date" ).toString() ;
				m_id           = object.value( "id" ).toString() ;

				if( !m_uploadDate.isEmpty() ){

					m_uploadDate = QObject::tr( "Upload Date:" ) + " " + m_uploadDate ;
				}

				m_intDuration = object.value( "duration" ).toInt() ;

				if( m_intDuration != 0 ){

					auto s = engines::engine::functions::timer::duration( m_intDuration * 1000 ) ;
					m_duration = QObject::tr( "Duration:" ) + " " + s ;
				}
			}
		}
		const QString& thumbnailUrl() const
		{
			return m_thumbnailUrl ;
		}
		const QString& title() const
		{
			return m_title ;
		}
		const QString& url() const
		{
			return m_url ;
		}
		QString uiText() const
		{
			auto title = [ & ](){

				if( m_title.isEmpty() || m_title == "\n" ){

					return m_url ;
				}else{
					return m_title ;
				}
			}() ;

			if( m_duration.isEmpty() ){

				if( m_uploadDate.isEmpty() ){

					return title ;
				}else{
					return m_uploadDate + "\n" + title ;
				}
			}else{
				if( m_uploadDate.isEmpty() ){

					return m_duration + "\n" + title ;
				}else{
					return m_duration + ", " + m_uploadDate + "\n" + title ;
				}
			}
		}
		const QString& uploadDate() const
		{
			return m_uploadDate ;
		}
		bool valid() const
		{
			return m_json ;
		}
		const QJsonDocument& doc() const
		{
			return m_json.doc() ;
		}
		QString errorString() const
		{
			return m_json.errorString() ;
		}
		const QString& duration() const
		{
			return m_duration ;
		}
		const QString& id() const
		{
			return m_id ;
		}
		int intDuration() const
		{
			return m_intDuration ;
		}
	private:
		QString m_thumbnailUrl ;
		QString m_title ;
		QString m_uploadDate ;
		QString m_url ;
		QString m_duration ;
		QString m_id ;
		int m_intDuration ;
		util::Json m_json ;
	};

	template< typename FinishedState >
	void updateFinishedState( const engines::engine& engine,
				  settings& s,
				  tableWidget& table,
				  const FinishedState& f )
	{
		const auto& index = f.index() ;
		const auto& es = f.exitState() ;

		f.setState( table.runningStateItem( index ) ) ;

		const auto backUpUrl = table.url( index ) ;

		auto& item = table.uiTextItem( index ) ;

		auto a = item.text() ;

		item.setText( engine.updateTextOnCompleteDownlod( a,backUpUrl,es ) ) ;

		if( !es.cancelled() ){

			if( es.success() ){

				engine.runCommandOnDownloadedFile( a,backUpUrl ) ;
			}

			if( f.allFinished() ){

				auto a = s.commandWhenAllFinished() ;

				if( !a.isEmpty() ){

					auto args = util::split( a,' ',true ) ;

					auto exe = args.takeAt( 0 ) ;

					QProcess::startDetached( exe,args ) ;
				}
			}
		}
	}

	template< typename Opts,typename Functions >
	class options
	{
	public:
		options( Opts opts,Functions functions ) :
			m_opts( std::move( opts ) ),
			m_functions( std::move( functions ) )
		{
		}
		void done( utility::ProcessExitState e )
		{
			m_functions.done( std::move( e ),m_opts ) ;
		}
		void listRequested( const QList< QByteArray >& e )
		{
			m_functions.list( e ) ;
		}
		bool listRequested()
		{
			return m_opts.listRequested ;
		}
		int index()
		{
			return m_opts.index ;
		}
		bool debug()
		{
			return m_opts.debug ;
		}
		void disableAll()
		{
			m_functions.disableAll( m_opts ) ;
		}
		QString downloadFolder() const
		{
			return utility::downloadFolder( m_opts.ctx ) ;
		}
		const QProcessEnvironment& processEnvironment() const
		{
			return utility::processEnvironment( m_opts.ctx ) ;
		}
	private:
		Opts m_opts ;
		Functions m_functions ;
	} ;

	template< typename List,typename DisableAll,typename Done >
	struct Functions
	{
		List list ;
		DisableAll disableAll ;
		Done done ;
	} ;

	template< typename List,typename DisableAll,typename Done >
	Functions< List,DisableAll,Done > OptionsFunctions( List list,DisableAll disableAll,Done done )
	{
		return { std::move( list ),std::move( disableAll ),std::move( done ) } ;
	}

	template< typename DisableAll,typename Done >
	auto OptionsFunctions( DisableAll disableAll,Done done )
	{
		auto aa = []( const QList< QByteArray >& ){} ;

		using type = Functions< decltype( aa ),DisableAll,Done > ;

		return type{ std::move( aa ),std::move( disableAll ),std::move( done ) } ;
	}
}

#endif
