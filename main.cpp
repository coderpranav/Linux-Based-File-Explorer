#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <termios.h>
#include <sys/ioctl.h>

using namespace std;

//defining macros
#define set_term_pos(x, y) printf("%c[%d;%dH", 27, x, y)
#define clear_screen() printf("%c[2J", 27)

//declaring all required variables.
string myroot;
vector<string> dir;
int no_of_files;
int row, col;
int flag_root = 0;
int x_cur = 1, y_cur = 58;
const char *curr_path;
int tracker;
stack<string> forward_container;
stack<string> backward_container;
vector<string> command;

void clear_line()
{
    int lastLine = row + 1;
    printf("%c[%d;%dH", 27, lastLine, 1);
    printf("%c[2K", 27);
    cout << ":";
}
void display_error(string str)
{
    //clearCommand();
    cout << endl;
    cout << "\033[0;32m" << str << endl;
    cout << "\033[0m";
    cout << ":";
}

int get_total_file_count()
{
    int total_records;
    struct winsize window;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &window);
    row = window.ws_row - 1;
    col = window.ws_col;
    if (no_of_files <= row)
    {
        total_records = no_of_files;
    }
    else
    {
        total_records = row;
    }
    return total_records;
}

void display_dirs(const char *dir_name, const char *root)
{

    string newpath;
    if (flag_root == 1)
    {
        newpath = string(dir_name);
    }
    else
    {
        newpath = string(root) + '/' + string(dir_name);
    }
    const char *path = newpath.c_str();
    struct stat st;
    if (stat(path, &st) == -1)
    {
        perror("lstat");
    }

    printf((S_ISDIR(st.st_mode)) ? "d" : "-");
    printf((st.st_mode & S_IRUSR) ? "r" : "-");
    printf((st.st_mode & S_IWUSR) ? "w" : "-");
    printf((st.st_mode & S_IXUSR) ? "x" : "-");
    printf((st.st_mode & S_IRGRP) ? "r" : "-");
    printf((st.st_mode & S_IWGRP) ? "w" : "-");
    printf((st.st_mode & S_IXGRP) ? "x" : "-");
    printf((st.st_mode & S_IROTH) ? "r" : "-");
    printf((st.st_mode & S_IWOTH) ? "w" : "-");
    printf((st.st_mode & S_IXOTH) ? "x" : "-");

    struct passwd *pw = getpwuid(st.st_uid);
    struct group *gr = getgrgid(st.st_gid);
    if (pw != 0)
        printf(" %8s", pw->pw_name);
    if (gr != 0)
        printf(" %8s", gr->gr_name);

    printf("%8.2fK", ((double)(st.st_size)) / 1024);
    char *mtime = (ctime(&st.st_mtime));
    string filetime = mtime;
    //tt[strlen(tt) - 1] = '\0';
    printf("%16s", filetime.substr(4, 12).c_str());
    if (S_ISDIR(st.st_mode))
    {
        printf("\033[1;34m");
        printf("\t%-15s\n", dir_name);
        printf("\033[0m");
    }
    else
        printf("\t%-15s\n", dir_name);
}
void open_dir(const char *path)
{

    struct dirent *d;
    DIR *dr = opendir(path);
    if (!dr)
    {
        int end_of_line = row + 1;
        printf("%c[%d;%dH", 27, end_of_line, 1);
        printf("%c[2K", 27);
        cout << ":";
        display_error("Ops something wrong to open Dir :::" + string(path));
        return;
    }
    dir.clear();
    int dcount = 0;
    if (dr)
    {
        while ((d = readdir(dr)) != NULL)
        {
            //cout<<d->d_name<<endl;
            if (!((string(d->d_name) == "..") && (strcmp(path, myroot.c_str()) == 0)))
            {
                dcount++;
                dir.push_back(d->d_name);
            }
        }
    }
    else
    {
        display_error("Unable to find Directory");
    }
    closedir(dr);
    no_of_files = dcount;
    int total_records = get_total_file_count();
    sort(dir.begin(), dir.end());
    tracker = 0;
    printf("\033[H\033[J");
    printf("%c[%d;%dH", 27, 1, 1);
    for (int i = 0, j = 1; i < no_of_files && j <= total_records; i++, j++)
    {
        //cout<<dir[i]<<endl;
        display_dirs(dir[i].c_str(), path);
    }
    display_error("My curr path----> " + string(path));
    //printf("%c[%d;%dH", 27, 1, 80);
}

void Empty_Container(stack<string> &st)
{

    while (!st.empty())
    {
        st.pop();
    }
}

void fix_back_path(string given_path)
{

    int len = given_path.size();
    int i = given_path.size() - 1;
    int count = 0;
    if (given_path == ".")
        return;
    while (i >= 0 && given_path[i] != '/')
    {
        count++;
        i--;
    }
    string result_path = given_path.substr(0, len - (count + 1));
    char *temp_path = new char[result_path.length()+1];
    strcpy(temp_path,result_path.c_str());
    curr_path = temp_path;
}
// void fix_back_path(const char *given_path){
//     size_t pos;
// 	string newPath;
// 	string tempPath = string(given_path);
// 	pos = tempPath.find_last_of("/\\");
// 	newPath = tempPath.substr(0, pos);
// 	curr_path = newPath.c_str();
// }

void copy_file(string sfile, string dfile)
{
    int fd, fdw;
    char c;
    //string tempdfile=dfile;
    const char *src_file, *dst_file;
    src_file = sfile.c_str();
    //cout<<dfile<<endl<<sfile;
    if (dfile[0] == '/')
    {
        dfile = dfile + "/" + sfile;
        dst_file = dfile.c_str();
    }

    else
        dst_file = dfile.c_str();

    fd = open(src_file, O_RDONLY);
    //DIR *dirp;
    //dirp = opendir(dst_file);
    fdw = open(dst_file, O_WRONLY | O_CREAT);
    //cout<<fdw<<endl;
    struct stat curr_file;
    stat(src_file, &curr_file);
    chown(dst_file, curr_file.st_uid, curr_file.st_gid);
    chmod(dst_file, curr_file.st_mode);
    while (read(fd, &c, 1))
    {
        //cout<<c;
        if (fd != -1)
            write(fdw, &c, 1);
    }
    //chmod(dst_file,S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
    close(fd);
    close(fdw);
    //closedir(dirp);
}

void create_file(string dfile, string fpath)
{

    int fd;
    fpath = fpath + "/" + dfile;
    const char *dst_file = fpath.c_str();

    fd = open(dst_file, O_WRONLY | O_CREAT);
    chmod(dst_file, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
    close(fd);
}
void move_file(string sfile, string dfile)
{
    int fd, fdw;
    char c;
    //string tempdfile=dfile;
    const char *src_file, *dst_file;
    src_file = sfile.c_str();
    //cout<<dfile<<endl<<sfile;
    if (dfile[0] == '/')
    {
        dfile = dfile + "/" + sfile;
        dst_file = dfile.c_str();
    }

    else
        dst_file = dfile.c_str();

    fd = open(src_file, O_RDONLY);
    //DIR *dirp;
    //dirp = opendir(dst_file);
    fdw = open(dst_file, O_WRONLY | O_CREAT);
    //cout<<fdw<<endl;
    struct stat curr_file;
    stat(src_file, &curr_file);
    chown(dst_file, curr_file.st_uid, curr_file.st_gid);
    chmod(dst_file, curr_file.st_mode);
    while (read(fd, &c, 1))
    {
        //cout<<c;
        if (fd != -1)
            write(fdw, &c, 1);
    }
    //chmod(dst_file,S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
    close(fd);
    close(fdw);
    //closedir(dirp);
    remove(src_file);
}

void create_dir(string dirname, string dirpath)
{

    dirpath = dirpath + "/" + dirname;
    const char *dirp = dirpath.c_str();
    mkdir(dirp, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
}
void delete_file(string dirpath)
{

    //dirpath = dirpath+"/"+dirname;
    const char *dirp = dirpath.c_str();
    //mkdir(dirp,S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
    remove(dirp);
}

int goto_dir(string dirpath)
{

    int d = chdir(dirpath.c_str());
    if (d == 0)
    {
        backward_container.push(string(curr_path));
        curr_path = ".";
        return 0;
    }
    return -1;
}
void delete_dir(string dirpath)
{

    struct dirent *d;
    DIR *dr = opendir(dirpath.c_str());
    if (dr)
    {
        while ((d = readdir(dr)) != NULL)
        {
            //cout<<d->d_name<<endl;
            if ((string(d->d_name) == "..") || (string(d->d_name) == "."))
            {
            }
            else
            {
                string newpath = dirpath + "/" + string(d->d_name);
                struct stat sb;
                if (stat(newpath.c_str(), &sb) == -1)
                {
                    perror("lstat");
                }
                else
                {
                    if ((S_ISDIR(sb.st_mode)))
                    {
                        delete_dir(newpath.c_str());
                    }
                    else
                    {
                        remove(newpath.c_str());
                    }
                }
            }
        }
        closedir(dr);
        int status = rmdir(dirpath.c_str());
        if (status == -1)
        {
            cout << "Error in deleting directory";
        }
    }
    else
    {
        cout << "Unable to find Directory";
    }
}

void start_command_mode()
{

    char keyb_inp;
    do
    {
        string icommand;
        while (((keyb_inp = getchar()) != 10) && keyb_inp != 27)
        {
            if (keyb_inp == 127)
            {
                clear_line();
                if (icommand.length() <= 1)
                {
                    icommand = "";
                }
                else
                {
                    icommand = icommand.substr(0, icommand.length() - 1);
                }
                cout << icommand;
            }
            else
            {
                icommand = icommand + keyb_inp;
                cout << keyb_inp;
            }
        }
        string temp = "";
        int i = 0;
        command.clear();
        // while (icommand[i] != '\0')
        while(i<icommand.size())
        {
            if (icommand[i] == ' ')
            {
                command.push_back(temp);
                //cout<<temp<<" ";
                temp = "";
            }
            else
            {
                temp += icommand[i];
            }
            i++;
        }
        command.push_back(temp);
        if (keyb_inp == 10)
        {

            if (command[0] == "copy")
            {
                //cout<<endl;
                if (command.size() < 3)
                    cout << "Too few arguments";
                else
                {
                    for (int i = 1; i < command.size() - 1; i++)
                    {
                        copy_file(command[i], command[command.size() - 1]);
                    }
                }
                clear_line();
            }

            else if (command[0] == "move")
            {
                //cout << "Move";
                if (command.size() < 3)
                    cout << "Too few arguments";
                else
                {
                    for (int i = 1; i < command.size() - 1; i++)
                    {
                        move_file(command[i], command[command.size() - 1]);
                    }
                }
                clear_line();
            }

            else if (command[0] == "rename")
            {
                //cout << "Rename";
                if (command.size() == 3)
                {
                    const char *old_file, *new_file;
                    old_file = command[1].c_str();
                    new_file = command[2].c_str();
                    rename(old_file, new_file);
                }
                clear_line();
            }

            else if (command[0] == "create_file")
            {
                //cout << "Create File";
                if (command.size() < 3)
                    cout << "Too few arguments";
                else
                {
                    for (int i = 1; i < command.size() - 1; i++)
                    {
                        create_file(command[i], command[command.size() - 1]);
                    }
                }
                clear_line();
            }
            else if (command[0] == "create_dir")
            {
                //cout << "Create dir";
                if (command.size() < 3)
                    cout << "Too few arguments";
                else
                {
                    for (int i = 1; i < command.size() - 1; i++)
                    {
                        create_dir(command[i], command[command.size() - 1]);
                    }
                }
                clear_line();
            }
            else if (command[0] == "delete_file")
            {
                //cout << "Delete_file";
                if (command.size() == 1)
                    cout << "Too few arguments";
                else
                {
                    delete_file(command[command.size() - 1]);
                }
                clear_line();
            }
            else if (command[0] == "delete_dir")
            {
                //cout << "delete_dir";
                if (command.size() == 1)
                    cout << "Too few arguments";
                else
                {
                    delete_dir(command[command.size() - 1]);
                }
                clear_line();
            }
            else if (command[0] == "goto")
            {
                //cout << "goto";
                if (command.size() == 1)
                    cout << "Too few arguments";
                else
                {
                    int val = goto_dir(command[command.size() - 1]);
                    if (val == 0)
                        break;
                }
                clear_line();
            }
            // if(command[0]=="search"){
            //     cout<<"Copy";
            // }
        }

    } while (keyb_inp != 27);
}
void navigate()
{

    curr_path = myroot.c_str();
    x_cur = 1;
    y_cur = 58;
    set_term_pos(x_cur, y_cur);
    char keyb_inp;
    struct termios cur_setting, new_setting;
    tcgetattr(STDIN_FILENO, &cur_setting);
    new_setting = cur_setting;
    new_setting.c_lflag = (new_setting.c_lflag & ~ICANON);
    new_setting.c_lflag = (new_setting.c_lflag & ~ECHO);

    if ((tcsetattr(STDIN_FILENO, TCSAFLUSH, &new_setting)))
    {

        display_error("setting attributes failed");
    }
    else
    {

        while (true)
        {

            int end_line = row + 1;
            set_term_pos(end_line, 0);
            cout << "##### Welcome to Normal Mode #####";
            set_term_pos(x_cur, y_cur);

            //take the keyboard input
            keyb_inp = cin.get();
            if (keyb_inp == 'q')
            {
                int lastthree = no_of_files + 4;
                printf("\033[%d;1H", lastthree);
                break;
            }
            //int ESC = 27;
            if (keyb_inp == 27)
            {

                keyb_inp = cin.get();
                keyb_inp = cin.get();

                //UP Arrow Pressed
                if (keyb_inp == 'A')
                {

                    if (x_cur + tracker > 1)
                    {

                        x_cur--;

                        if (x_cur > 0)
                            set_term_pos(x_cur, y_cur);

                        else if (x_cur <= 0 && x_cur + tracker >= 1)
                        {

                            clear_screen();
                            if (tracker > 0)
                                tracker--;

                            set_term_pos(1, 1);

                            for (int i = tracker; i <= row + tracker - 1; i++)
                            {

                                display_dirs(dir[i].c_str(), curr_path);
                            }
                            x_cur++;
                            set_term_pos(x_cur, y_cur);
                        }
                    }
                }
                //Down Arrow Pressed
                else if (keyb_inp == 'B')
                {
                    //cout<<"Came Here";
                    int total_records;
                    if ((x_cur + tracker) < (no_of_files))
                    {

                        x_cur++;
                        if (x_cur <= row)
                            set_term_pos(x_cur, y_cur);
                        else if (x_cur > row && x_cur + tracker <= no_of_files)
                        {

                            clear_screen();
                            total_records = get_total_file_count() - 1;

                            if (no_of_files > row)
                                tracker++;

                            set_term_pos(1, 1);

                            for (int i = tracker; i <= total_records + tracker; i++)
                            {

                                display_dirs(dir[i].c_str(), curr_path);
                            }
                            x_cur--;
                        }
                        set_term_pos(x_cur, y_cur);
                    }
                }

                //LEFT Arrow Pressed
                else if (keyb_inp == 'D')
                {

                    if (!backward_container.empty())
                    {
                        if (!flag_root)
                        {
                            forward_container.push(string(curr_path));
                        }
                        string top = backward_container.top();
                        backward_container.pop();
                        curr_path = top.c_str();
                        flag_root = 0;
                        open_dir(curr_path);
                        x_cur = 1, y_cur = 58;
                        set_term_pos(x_cur, y_cur);
                    }
                }
                //Right Arrow Pressed
                else if (keyb_inp == 'C')
                {

                    if (!forward_container.empty())
                    {
                        if (!flag_root)
                        {
                            backward_container.push(string(curr_path));
                        }
                        string top = forward_container.top();
                        forward_container.pop();
                        curr_path = top.c_str();
                        flag_root = 0;
                        open_dir(curr_path);
                        x_cur = 1, y_cur = 58;
                        set_term_pos(x_cur, y_cur);
                    }
                }
            }
            //Enter Key pressed
            else if (keyb_inp == 10)
            {

                string curr_dir = dir[x_cur + tracker - 1];
                string req_path;
                // if (flag_root == 1)
                //     req_path = curr_dir;
                // else
                req_path = string(curr_path) + "/" + curr_dir;

                char *main_path = new char[req_path.length()+1];
                //req_path.c_str();
                strcpy(main_path,req_path.c_str());
                struct stat sb;
                stat(main_path, &sb);

                if ((sb.st_mode & S_IFMT) == S_IFDIR)
                {
                    //flag_root = 0;
                    x_cur = 1;
                    if (curr_dir == ".")
                    {
                    }
                    else if (curr_dir == "..")
                    {

                        backward_container.push(string(curr_path));
                        //fix_back_path(string(curr_path));
                        fix_back_path(string(curr_path));
                        Empty_Container(forward_container);
                    }
                    else
                    {
                        if (curr_path)
                        {
                            backward_container.push(string(curr_path));
                            Empty_Container(forward_container);
                        }
                        curr_path = main_path;
                    }
                    open_dir(curr_path);
                }
                else if ((sb.st_mode & S_IFMT) == S_IFREG)
                {
                    //code to be written
                    int open_file = open("/dev/null", O_WRONLY);
                    dup2(open_file, 2);
                    close(open_file);
                    pid_t processID = fork();
                    if (processID == 0)
                    {
                        execlp("xdg-open", "xdg-open", main_path, NULL);
                        exit(0);
                    }
                    // else
                    //     display_error("Unknown File !!! :::::" + string(curr_dir));
                }
            }
            //h key pressed
            else if (keyb_inp == 104)
            {
                if (string(curr_path) != myroot)
                {
                    if (!flag_root)
                        backward_container.push(string(curr_path));
                    Empty_Container(forward_container);
                    flag_root = 0;
                    curr_path = myroot.c_str();
                    open_dir(curr_path);
                    x_cur = 1, y_cur = 58;
                    set_term_pos(x_cur, y_cur);
                }
            }
            //Backspace key Pressed
            else if (keyb_inp == 127)
            {
                if ((string(curr_path) != myroot) && !flag_root)
                {
                    backward_container.push(curr_path);
                    Empty_Container(forward_container);
                    fix_back_path(string(curr_path));
                    open_dir(curr_path);
                    x_cur = 1, y_cur = 58;
                    set_term_pos(x_cur, y_cur);
                }
            }
            //COLON Pressed
            else if (keyb_inp == 58)
            {
                int end_line_com = row + 1;
                set_term_pos(end_line_com, 1);
                printf("\033[2K");
                cout << ":";
                //Start Command MODE
                start_command_mode();
            }
        }
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &cur_setting);
}

int main(int argc, char *argv[])
{

    printf("\033[8;80;140t");
    //cout << argc;
    if (argc == 1)
    {
        string curr = ".";
        myroot = curr;
        open_dir(curr.c_str());
    }
    else
    {
        myroot = argv[1];
        open_dir(myroot.c_str());
    }

    navigate();

    return 0;
}