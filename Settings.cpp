#include "stdafx.h"
#include "Settings.h"

#include "../lsMisc/CommandLineParser.h"
#include "../lsMisc/stdwin32/stdwin32.h"



namespace Ambiesoft {
	namespace FolderConfig {

		using namespace stdwin32;

		using namespace System::IO;
		using namespace System::Text;
		using namespace System::Windows::Forms;

		String^ getHelpMessage()
		{
			StringBuilder sb;
			sb.AppendLine(I18N(L"Usage:"));
			sb.AppendLine(L"FolderConfig.exe [/title Title] [/inifile Inifile] [/defaultpath Defaultpath] /creator Creator /appname Appname [/section Section]");
			sb.AppendLine();

			sb.AppendLine(I18N(L"Title: Title shown in Titlebar."));
			sb.AppendLine(I18N(L"Inifile: filename user chosen data will be saved. (Default is folder.ini)"));
			sb.AppendLine(I18N(L"Defaultpath: Default relative folder path from exe-resident folder. (Default is Exe-resident folder)"));
			sb.AppendLine(I18N(L"Creator: Creator name used as subfolder of roaming or local folder."));
			sb.AppendLine(I18N(L"Appname: App name used as subfolder of Creator."));
			sb.AppendLine(I18N(L"Section: Section name of config. (Default is FolderConfig)"));

			return sb.ToString();
		}

		void ErrorExit(String^ s)
		{
			MessageBox::Show(s + "\r\n\r\n" + getHelpMessage(),
				Application::ProductName,
				MessageBoxButtons::OK,
				MessageBoxIcon::Error);

			System::Environment::Exit(1);
		}
		void Alert(String^ msg)
		{
			MessageBox::Show(msg,
				Application::ProductName,
				MessageBoxButtons::OK,
				MessageBoxIcon::Warning);
		}
		void Alert(Exception^ ex)
		{
			Alert(ex->Message);
		}
		bool Settings::init()
		{
			CCommandLineParser parser(CaseFlag_CaseInsensitive);
			COption opTitle(L"/title", 1);
			parser.AddOption(&opTitle);

			//COption opIniFile(L"/inifile", 1);
			//parser.AddOption(&opIniFile);

			COption opDefaultPath(L"/defaultpath", 1);
			parser.AddOption(&opDefaultPath);

			COption opCreator(L"/creator", 1);
			parser.AddOption(&opCreator);

			COption opAppName(L"/appname", 1);
			parser.AddOption(&opAppName);

			COption opSection(L"/section", 1);
			parser.AddOption(&opSection);

			COption opCulture(L"/culture", 1);
			parser.AddOption(&opCulture);

			COption opHelp(L"/h", L"/?");
			parser.AddOption(&opHelp);


			parser.Parse();


			// First, set culture for I18N
			if(opCulture.hadOption())
			{
				if(opCulture.getValueCount() > 1)
				{
					ErrorExit(I18N(String::Format(L"{0} cultures specified. Only one acceptable.",
						opSection.getValueCount())));
				}
				String^ cultureName = gcnew String(opCulture.getFirstValue().c_str());

				try
				{
					System::Globalization::CultureInfo^ ci = gcnew System::Globalization::CultureInfo(cultureName);
					System::Threading::Thread::CurrentThread->CurrentCulture = ci;
					System::Threading::Thread::CurrentThread->CurrentUICulture = ci;
				}
				catch(Exception^ ex)
				{
					Alert(ex);
				}
			}

			HashIni^ ini = Profile::ReadAll(Path::Combine(Path::GetDirectoryName(Application::ExecutablePath),
				Application::ProductName+L".ini"));
			
			Profile::GetString("Option", "appname", nullptr, appName_, ini);
			Profile::GetString("Option", "creator", nullptr, creator_, ini);

			if(opHelp.hadOption())
			{
				MessageBox::Show(getHelpMessage(),
					Application::ProductName,
					MessageBoxButtons::OK,
					MessageBoxIcon::Information
					);
				return 0;
			}

			if (opTitle.hadOption())
			{
				Settings::title_ = gcnew String(opTitle.getValueStrings().c_str());
			}
			if (String::IsNullOrEmpty(Settings::title_))
				Settings::title_ = Application::ProductName;

			// save file
			//if (opIniFile.hadOption())
			//{
			//	if (opIniFile.getValueCount() > 1)
			//	{
			//		ErrorExit(I18N(String::Format(L"{0} ini files specified. Only one acceptable.",
			//			opIniFile.getValueCount())));
			//	}


			//	if (!IsFullPathNamble(opIniFile.getValueStrings().c_str()))
			//	{
			//		ErrorExit(I18N(String::Format(L"{0} must not include {1}.",
			//			toCLR(opIniFile.getValueStrings()), GetFullPathInamableCharsCLR())));
			//	}

			//	String^ dir = System::IO::Directory::GetParent(System::Windows::Forms::Application::ExecutablePath)->FullName;
			//	String^ file = gcnew String(opIniFile.getValueStrings().c_str());
			//	String^ fullpath = System::IO::Path::Combine(dir, file);
			//	Settings::iniFileName_ = fullpath;
			//}
			//else
			//{
			//	Settings::iniFileName_ = Settings::DefaultInifileName;
			//}
			//if (String::IsNullOrEmpty(Settings::iniFileName_))
			//{
			//	ErrorExit(I18N(L"No ini file specified"));
			//}


			// default path
			if (opDefaultPath.hadOption())
			{
				if (opDefaultPath.getValueCount() > 1)
				{
					ErrorExit(I18N(String::Format(L"{0} default paths specified. Only one acceptable.",
						opDefaultPath.getValueCount())));
				}
				if (!IsFileNamble(opDefaultPath.getValueStrings().c_str()))
				{
					ErrorExit(I18N(String::Format(L"{0} must not include {1}.",
						"defaultpath", GetFileInamableCharsCLR())));
				}
				Settings::defaultUserPath_ = gcnew String(opDefaultPath.getValueStrings().c_str());
			}
			// default path is nullable


			// creator
			if (opCreator.hadOption())
			{
				if (opCreator.getValueCount() > 1)
				{
					ErrorExit(I18N(String::Format(L"{0} creators path specified. Only one acceptable.",
						opCreator.getValueCount())));
				}
				if (!IsFileNamble(opCreator.getValueStrings().c_str()))
				{
					ErrorExit(I18N(String::Format(L"{0} must not include {1}.",
						"creator", GetFileInamableCharsCLR())));
				}
				Settings::creator_ = gcnew String(opCreator.getValueStrings().c_str());
			}
			//else
			//{
			//	Settings::creator_ = Settings::DefaultCreator;
			//}

			if (String::IsNullOrEmpty(Settings::Creator))
			{
				ErrorExit(I18N(L"Creator must be specified."));
			}


			// appname
			if (opAppName.hadOption())
			{
				if (opAppName.getValueCount() > 1)
				{
					ErrorExit(I18N(String::Format(L"{0} appnames specified. Only one acceptable.",
						opAppName.getValueCount())));
				}
				if (!IsFileNamble(opAppName.getValueStrings().c_str()))
				{
					ErrorExit(I18N(String::Format(L"{0} must not include {1}.",
						L"appname", GetFileInamableCharsCLR())));
				}
				Settings::appName_ = gcnew String(opAppName.getValueStrings().c_str());
			}
			//else
			//{
			//	Settings::appName_ = Settings::DefaultAppName;
			//}
			if (String::IsNullOrEmpty(Settings::AppName))
			{
				ErrorExit(I18N(L"Appname must be specified."));
			}


			// section
			if (opSection.hadOption())
			{
				if (opSection.getValueCount() > 1)
				{
					ErrorExit(I18N(String::Format(L"{0} appnames specified. Only one acceptable.",
						opSection.getValueCount())));
				}
				Settings::section_ = gcnew String(opSection.getValueStrings().c_str());
			}
			if (String::IsNullOrEmpty(Settings::Section))
			{
				Settings::section_ = L"FolderConfig";
			}



			// check unknown option
			if (parser.hadUnknownOption())
			{
				StringBuilder sb;
				sb.AppendLine(I18N(L"Unknown option:"));
				sb.AppendLine(gcnew String(parser.getUnknowOptionStrings().c_str()));
				ErrorExit(sb.ToString());
			}

			return true;

		}

		//String^ Settings::InifileName::get()
		//{
		//	DASSERT(!String::IsNullOrEmpty(iniFileName_));
		//	DASSERT(stdIsFullPath(toLPCW(iniFileName_)));

		//	return iniFileName_;
		//}

		String^ Settings::DefaultUserPath::get()
		{
			if (String::IsNullOrEmpty(defaultUserPath_))
				return toCLR(stdGetParentDirectory(stdGetModuleFileName()));

			if (!stdIsFullPath(toLPCW(defaultUserPath_)))
				return toCLR(stdCombinePath(stdGetParentDirectory(stdGetModuleFileName()), toWstring(defaultUserPath_)));

			return defaultUserPath_;
		}

	}
}