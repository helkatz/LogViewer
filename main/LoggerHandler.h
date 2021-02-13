#pragma once
#include <core/common.h>
#include <QSqlField>
#include <qthread.h>
#include <mutex>
#include <qlogging.h>
#include <qdebug.h>
#include <qsqlerror.h>
#include <qsqldriver.h>
#include <common/Logger.h>
#include <qfile.h>
//Q_DECLARE_METATYPE(boost::chrono::system_clock::time_point)
namespace logger {

	class LogfileMessageHandler : public QThread, public logger::Logger::MessageHandler
	{
		Q_OBJECT;
		QString fileName;
		std::vector<Message> messageQueue;
		std::mutex mutex;
		QFile qtf;
		private slots:
		void run()
		{
			
			while (isRunning()) {
				QThread::msleep(500);
				{
					std::lock_guard<std::mutex> lock(mutex);
					if (messageQueue.size() == 0)
						continue;
					//auto f = fopen(fileName.c_str(), "a+");
					for (auto msg : messageQueue) {
						std::stringstream ss;
						ss
							<< boost::chrono::time_point_cast<boost::chrono::nanoseconds>(msg.time)
							<< "|" << msg.level
							<< "|" << msg.context.function
							<< "|" << msg.message
							<< "\n"
							;
						qtf.write(ss.str().c_str(), ss.tellp());
						//fwrite(ss.str().c_str(), 1, ss.tellp(), f);
					}
					//fclose(f);
					messageQueue.clear();
				}
			}
		}
	public:
		LogfileMessageHandler() :
			QThread()
		{
			moveToThread(this);
			
			start();
		}

		bool init(const QString& logFile)
		{
			if (logFile == 0)
				return false;
			fileName = logFile;
			return qtf.open(fopen(fileName.toStdString().c_str(), "a+"), QIODevice::Append);
		}

		void do_handle(Message& msg)
		{
			//std::lock_guard<std::mutex> lock(mutex);
			//messageQueue.push_back(msg);

			//auto f = fopen(fileName.c_str(), "a+");
			std::stringstream ss;
			ss
				<< boost::chrono::time_point_cast<boost::chrono::nanoseconds>(msg.time)
				<< "|" << msg.level
				<< "|" << msg.context.function
				<< "|" << msg.message
				<< "\n"
				;
			qtf.write(ss.str().c_str(), ss.tellp());
			qtf.flush();
			//fwrite(ss.str().c_str(), 1, ss.tellp(), f);

		}
	};
}
