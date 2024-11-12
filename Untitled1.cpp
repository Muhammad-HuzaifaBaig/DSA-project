#include <bits/stdc++.h>
#include <windows.h>
namespace fs = std::filesystem;
using namespace std;

class Repository {
	public:
		void init() {
			if (fs::exists(".myvcs")) {
				cout << "Already Initialized myvcs directory\n";
			} else {
				try {
					fs::create_directory(".myvcs");
					fs::create_directory(".myvcs/AllCommitsfiles");
					fs::create_directory(".myvcs/AllCommithistory");
					fs::create_directory(".myvcs/StagingArea");

					if (SetFileAttributes(".myvcs", FILE_ATTRIBUTE_HIDDEN) == 0) {
						cerr << "Failed to set .myvcs as hidden. Error code: " << GetLastError() << '\n';
					} else {
						cout << "Initialized hidden myvcs directory\n";
					}
				} catch (const fs::filesystem_error& e) {
					cerr << "Filesystem error: " << e.what() << '\n';
				}
			}
		}

		auto add(const string& filename) {
			string stagingPath = ".myvcs/StagingArea/" + fs::path(filename).filename().string();
			string commitFile = ".myvcs/AllCommitsfiles/" + fs::path(filename).filename().string();

			int version = 1;
			while (fs::exists(commitFile)) {
				commitFile = ".myvcs/AllCommitsfiles/" + filename.substr(0, filename.find_last_of('.')) + "_v" + to_string(version++) + filename.substr(filename.find_last_of('.'));
			}
			if(version > 2) {
				commitFile = ".myvcs/AllCommitsfiles/" + filename.substr(0, filename.find_last_of('.')) + "_v" + to_string(--version) + filename.substr(filename.find_last_of('.'));
				commitFile = ".myvcs/AllCommitsfiles/" + filename.substr(0, filename.find_last_of('.')) + "_v" + to_string(--version) + filename.substr(filename.find_last_of('.'));
			} else
				commitFile = ".myvcs/AllCommitsfiles/" + fs::path(filename).filename().string();
			try {
				if (fs::exists(commitFile) && fs::file_size(filename) == fs::file_size(commitFile) && compareFiles(filename, commitFile)) {
					cout << "File '" << filename << "' is not modified. It cannot be staged again.\n";
					return;
				} else if(fs::exists(stagingPath)) {
					cout << "Already in staging directory.\n";
				} else {
					fs::copy(filename, stagingPath, fs::copy_options::overwrite_existing);
					cout << "File '" << filename << "' added to the Staging Area.\n";
				}
			} catch (const fs::filesystem_error& e) {
				cout << "Error while adding file: " << e.what() << endl;
			}
		}



		void commit() {
			try {
				if (fs::is_empty(".myvcs/StagingArea")) {
					cout << "No files in the Staging Area to commit.\n";
					return;
				}

				// Get the first file from StagingArea
				for (const auto& entry : fs::directory_iterator(".myvcs/StagingArea")) {
					if (fs::is_regular_file(entry)) {
						string stagingFile = entry.path().string();
						string filename = fs::path(stagingFile).filename().string();
						string commitFile = ".myvcs/AllCommitsfiles/" + filename;

						// Check if the file exists in AllCommitsfiles (for versioning)
						int version = 1;
						while (fs::exists(commitFile)) {
							commitFile = ".myvcs/AllCommitsfiles/" + filename.substr(0, filename.find_last_of('.')) + "_v" + to_string(version++) + filename.substr(filename.find_last_of('.'));
						}

						// Move the file from StagingArea to AllCommitsfiles (versioned if needed)
						fs::rename(stagingFile, commitFile);
						cout << "File '" << entry.path().filename().string() << "' committed as " << commitFile << ".\n";

						// Remove the file from StagingArea after committing
						fs::remove(entry.path());

						// Log the commit message (with a timestamp for uniqueness)
						string commitMessage;
						cout << "Enter commit message for this file: ";
						getline(cin >> ws, commitMessage);  // Read full commit message

						// Log the commit details
						string timestamp = to_string(time(0)); // Use Unix timestamp as commit ID
						string commitLogFile = ".myvcs/AllCommithistory/commit_" + timestamp + ".log";
						ofstream log(commitLogFile);
						if (log.is_open()) {
							log << "Commit Message: " << commitMessage << "\n";
							log << "Committed File: " << commitFile << "\n";
							log.close();
						} else {
							cout << "Error logging commit details.\n";
						}

						// Optionally break after committing one file, waiting for the next commit
						cout << "One file committed successfully.\n";
						break; // Remove this if you want to commit multiple files in one go
					}
				}
			} catch (const fs::filesystem_error& e) {
				cout << "Error during commit: " << e.what() << endl;
			}
		}

		void status() {
			try {
				bool fileFound = false;

				// Loop through all files in the main directory
				for (const auto& entry : fs::directory_iterator(".")) {
					if (fs::is_regular_file(entry)) {
						string filename = entry.path().filename().string();
						string commitFile = ".myvcs/AllCommitsfiles/" + fs::path(filename).filename().string();

						int version = 1;
						while (fs::exists(commitFile)) {
							commitFile = ".myvcs/AllCommitsfiles/" + filename.substr(0, filename.find_last_of('.')) + "_v" + to_string(version++) + filename.substr(filename.find_last_of('.'));
						}
						if(version > 2) {
							commitFile = ".myvcs/AllCommitsfiles/" + filename.substr(0, filename.find_last_of('.')) + "_v" + to_string(--version) + filename.substr(filename.find_last_of('.'));
							commitFile = ".myvcs/AllCommitsfiles/" + filename.substr(0, filename.find_last_of('.')) + "_v" + to_string(--version) + filename.substr(filename.find_last_of('.'));
						} else
							commitFile = ".myvcs/AllCommitsfiles/" + fs::path(filename).filename().string();

						// Check if the file exists in AllCommitsfiles
						if (fs::exists(commitFile)) {
							// Compare the files
							if (fs::file_size(entry.path()) != fs::file_size(commitFile) || !compareFiles(entry.path(), commitFile)) {
								cout << "Modified: " << filename << "\n";
							} else {
								cout << "UnModified: " << filename << "\n";
							}
						} else {
							cout << "Untracked: " << filename << "\n";
						}
						fileFound = true;
					}
				}

				// If no files found in the main directory
				if (!fileFound) {
					cout << "No files found in the main directory.\n";
				}
			} catch (const fs::filesystem_error& e) {
				cout << "Error during status check: " << e.what() << endl;
			}
		}
		void log() {
			for (const auto& entry : fs::directory_iterator(".myvcs/AllCommithistory")) {
				if (fs::is_regular_file(entry)) {
					string logFile = entry.path().string();
					ifstream f(logFile);
					string line;
					while(getline(f,line)){
						cout << line<<endl;
					}
					cout<<"\n---------------"<<endl;
				}
			}
		}
private:
				bool compareFiles(const fs::path& file1, const fs::path& file2) {
					ifstream f1(file1, ios::binary);
					ifstream f2(file2, ios::binary);

					if (!f1 || !f2) {
						return false; // Error opening files
					}

					return equal(istreambuf_iterator<char>(f1), istreambuf_iterator<char>(), istreambuf_iterator<char>(f2));
				}
			};

			int main() {
				Repository Repo;
				while (true) {
					string command, filename;
					cout << "Enter command (init, add <filename>, commit, status,log, exit): ";
					cin >> command;

					if (command == "init") {
						Repo.init();
					} else if (command == "add") {
						cin >> filename;
						Repo.add(filename);
					} else if (command == "commit") {
						Repo.commit();
					} else if (command == "status") {
						Repo.status();
					} 
					else if(command == "log"){
						Repo.log();
					}else if (command == "exit") {
						cout << "Exiting...\n";
						break;
					} else {
						cout << "Unknown command.\n";
					}
				}

				return 0;
			}
