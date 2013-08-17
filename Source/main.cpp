#include <iostream>
#include <list>
#include <sstream>

#include <stdlib.h>
#include <unistd.h>

// must be above QT stuff, else error
#include <libnotify/notify.h>

#include <QApplication>
#include <QDesktopWidget>
#include <QPixmap>
#include <QMessageBox>
#include <QSettings>
#include <QDir>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QClipboard>
 
// CURL stuff
#include <curl/curl.h>
#include <sys/stat.h>
#include <fcntl.h>



using namespace std;

QSettings* pSettings;

string GetTemporayFile(const string& name)
{
	size_t current = 0;
	
	while(true)
	{
		stringstream ss;
		
		ss << QDir::tempPath().toStdString();
		ss << "/" << current++ << "." << name;
		
		if(0 == access(ss.str().c_str(), 0))
			continue;
		
		return ss.str();
	}
}

void ErrorBox(const string& msg)
{
	QMessageBox::critical(nullptr, "Error", msg.c_str(), QMessageBox::Ok);
}

void OpenImage(const string& file)
{
	string launcher = pSettings->value("image_editor").toString().toStdString();
	
	if(launcher == "")
	{
		pSettings->setValue("image_editor", "xdg-open");
		launcher = "xdg-open";
	}
	
	system((launcher + " " + file).c_str());
}

size_t write_to_string(void *ptr, size_t size, size_t count, void *stream) 
{
  ((string*)stream)->append((char*)ptr, 0, size*count);
  return size*count;
}

void ActionCallback(NotifyNotification *notification, char *action, gpointer user_data)
{
	
}

bool Upload(const string& file)
{
	CURL *curl;
	CURLcode res;
	struct stat file_info;
	double speed_upload, total_time;
	FILE *fd;

	fd = fopen(file.c_str(), "rb"); /* open file to upload */ 
	if(!fd)
		return false;
	
	if(fstat(fileno(fd), &file_info) != 0)
		return 1;
		
	curl = curl_easy_init();
	
	
	
	struct curl_httppost *formpost=NULL;
	struct curl_httppost *lastptr=NULL;
	struct curl_slist *headerlist=NULL;
	static const char buf[] = "Expect:";
	curl_global_init(CURL_GLOBAL_ALL);

	/* Fill in the file upload field */ 
	curl_formadd(&formpost,
			   &lastptr,
			   CURLFORM_COPYNAME, "file",
			   CURLFORM_FILE, file.c_str(),
			   CURLFORM_END);
	
	if(!curl)
	{
		curl_easy_cleanup(curl);
		return false;
	}
	
	headerlist = curl_slist_append(headerlist, buf);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
	
	string user = pSettings->value("user").toString().toStdString();
	string auth = pSettings->value("auth").toString().toStdString();
	
	char* puser = curl_easy_escape(curl, user.c_str(), user.length());
	char* pauth = curl_easy_escape(curl, auth.c_str(), auth.length());
	
	user = puser;
	auth = pauth;
	
	curl_free(puser);
	curl_free(pauth);
	
	string towhom = pSettings->value("upload_location", "http://screenshot.xiatek.org/").toString().toStdString();
	
	curl_easy_setopt(curl, CURLOPT_URL, (towhom + "upload.php?user=" + user + "&auth=" + auth).c_str());
	
	string response;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
	
	res = curl_easy_perform(curl);
	
	
	
	if(res != CURLE_OK)
	{
		string err = curl_easy_strerror(res);
		ErrorBox(err);
		curl_easy_cleanup(curl);
		curl_formfree(formpost);
		curl_slist_free_all (headerlist);
		return false;
	}
	
	curl_easy_cleanup(curl);
	curl_formfree(formpost);
	curl_slist_free_all (headerlist);

	
	if(response.find("error") != string::npos)
	{
		ErrorBox(response);
		return false;
	}
	
	response = towhom + response;
	
	QClipboard* clipboard = QApplication::clipboard();
	clipboard->setText(response.c_str());
	
	
	
	//ErrorBox("Success: " + response);
	
	NotifyNotification* n;
	notify_init("Screenshot Uploaded");
	n = notify_notification_new ("Screenshot Uploaded", ("The screenshot has been uploaded to\n " + response).c_str(), pSettings->value("notification/icon", "video-display").toString().toStdString().c_str());
	notify_notification_set_timeout(n, 3000); //3 seconds
	
	if(pSettings->value("notification/critical", true).toBool())
		notify_notification_set_urgency(n, NOTIFY_URGENCY_CRITICAL);
	
	//void* x = malloc(0);
	notify_notification_add_action(n, "close", "Close", &ActionCallback, 0, 0);
	
	notify_notification_show (n, NULL);
}

bool Screenshot()
{
	QPixmap pixmap = QPixmap::grabWindow(QApplication::desktop()->winId());
	
	string file = GetTemporayFile("screenshot.png");
	
	cout << "editing: " << file << "\n";
	cout << pixmap.save(file.c_str(), "png", -1) << " : \n";
	
	OpenImage(file);
	
	QMessageBox::StandardButton reply = QMessageBox::question(nullptr, "Upload", "Do you want to upload the screenshot?", QMessageBox::Yes | QMessageBox::No);
	
	if (reply == QMessageBox::Yes)
		return Upload(file);
	return true;
}




int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	
	QSettings settings("ScreenShotter", "settings");
	pSettings = &settings;
	
	
	string user = pSettings->value("user").toString().toStdString();
	string auth = pSettings->value("auth").toString().toStdString();
	
	if(user == "" || auth == "")
	{
		pSettings->setValue("user", ""); // so they exist in the config
		pSettings->setValue("auth", "");
		
		ErrorBox("The user or auth code hasn't been set yet!");
		return 1;
	}
	
	if(!Screenshot())
		return 1;
	
	return 0;
}
