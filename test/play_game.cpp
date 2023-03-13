#include <ext/stdio_filebuf.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <iostream>
#include <charconv>
#include <chrono>
#include <memory>

class Pipe
{
public:
	Pipe();
	~Pipe();

	int readFD() const;
	int writeFD() const;

	void close() const;
private:
	int m_Read;
	int m_Write;
};

Pipe::Pipe()
{
	int fd[2];
	if (pipe(fd))
	{
		std::cerr << "Pipe failed to create" << std::endl;
		exit(1);
	}
	m_Read = fd[0];
	m_Write = fd[1];
}

Pipe::~Pipe()
{
	close();
}

void Pipe::close() const
{
	::close(m_Read);
	::close(m_Write);
}

int Pipe::readFD() const
{
	return m_Read;
}

int Pipe::writeFD() const
{
	return m_Write;
}

class Subprocess
{
public:
	Subprocess(const char* path);

	std::istream& stdout();
	std::ostream& stdin();
private:
    std::unique_ptr<__gnu_cxx::stdio_filebuf<char>> m_WriteBuf = nullptr; 
    std::unique_ptr<__gnu_cxx::stdio_filebuf<char>> m_ReadBuf = nullptr;
	std::istream m_Stdout;
	std::ostream m_Stdin;
	Pipe m_ReadPipe;
	Pipe m_WritePipe;
	pid_t m_Child;
};

Subprocess::Subprocess(const char* path)
	: m_Stdout(nullptr), m_Stdin(nullptr)
{
	int pid = fork();
	if (pid == -1)
	{
		std::cerr << "Fork failed" << std::endl;
		exit(1);
	}
	if (pid == 0)
	{
		dup2(m_ReadPipe.writeFD(), STDOUT_FILENO);
		dup2(m_WritePipe.readFD(), STDIN_FILENO);
		m_ReadPipe.close();
		m_WritePipe.close();
		int result = execlp(path, path);

		if (result == -1)
		{
			std::cerr << "Failed to exec program" << std::endl;
			exit(1);
		}
	}
	else
	{
		close(m_WritePipe.readFD());
		close(m_ReadPipe.writeFD());
		m_WriteBuf = std::make_unique<__gnu_cxx::stdio_filebuf<char>>(m_WritePipe.writeFD(), std::ios::out);
		m_ReadBuf = std::make_unique<__gnu_cxx::stdio_filebuf<char>>(m_ReadPipe.readFD(), std::ios::in);
		m_Child = pid;
		m_Stdin.rdbuf(m_WriteBuf.get());
		m_Stdout.rdbuf(m_ReadBuf.get());
	}
}

std::istream& Subprocess::stdout()
{
	return m_Stdout;
}

std::ostream& Subprocess::stdin()
{
	return m_Stdin;
}

constexpr int CHECKMATE = -1000000;

void myTerminate()
{
	std::cout << "terminate" << std::endl;
}

int main()
{
    // const char* const argv[] = {"./main", (const char*)0};
	std::set_terminate(myTerminate);
	Subprocess white("./main");
	Subprocess black("./main");

	using namespace std::chrono_literals;

	std::string allMoves;

	std::chrono::nanoseconds whiteClock = 1s;
	std::chrono::nanoseconds blackClock = 1s;
	std::chrono::nanoseconds increment = 50ms;

	while (true)
	{
		// white
		uint64_t time;
		white.stdin() << "eval 23i" << std::endl;
		std::string result[7];
		for (int i = 0; i < 7; i++)
		{
			std::getline(white.stdout(), result[i]);
			if (result[i] == "No move found")
			{
				std::cout << "\n\nDraw" << std::endl;
				goto end;
			}
		}
		/*for (int i = 0; i < 7; i++)
		{
			std::cout << "RRR: " << result[i] << std::endl;
		}*/

		int eval;
		std::from_chars(result[1].data() + 6, result[1].data() + result[1].length(), eval);
		if (eval == CHECKMATE)
		{
			std::cout << "\n\nWhite has been checkmated" << std::endl;
			break;
		}

		std::string_view move(result[2].begin() + 11, result[2].end());
		if (move[0] == 'a' && move[1] == '1' && move[2] == 'a' && move[3] == '1')
		{
			std::cout << "Draw" << std::endl;
		}
		
		std::from_chars(result[4].data() + 10, result[4].data() + result[4].length(), time);

		std::chrono::nanoseconds elapsed = std::chrono::nanoseconds(time);
		if (elapsed > whiteClock + increment)
		{
			std::cout << "White has run out of time" << std::endl;
			break;
		}

		whiteClock -= elapsed;
		whiteClock += increment;
		
		white.stdin() << "move " << move << std::endl;
		black.stdin() << "move " << move << std::endl;

		allMoves += move;
		allMoves += ' ';

		std::cout << "------WHITE------" << std::endl;
		std::cout << result[0] << std::endl;
		std::cout << "eval: " << eval << std::endl;
		std::cout << "move: " << move << std::endl;
		std::cout << "elapsed: " << elapsed.count() << std::endl;;

		
		// black
		black.stdin() << "eval 23i" << std::endl;
		for (int i = 0; i < 7; i++)
		{
			std::getline(black.stdout(), result[i]);
			if (result[i] == "No move found")
			{
				std::cout << "\n\nDraw" << std::endl;
				goto end;
			}
		}
	
		/*for (int i = 0; i < 7; i++)
		{
			std::cout << "RRR: " << result[i] << std::endl;
		}*/
		
		std::from_chars(result[1].data() + 6, result[1].data() + result[1].length(), eval);
		if (eval == CHECKMATE)
		{
			std::cout << "\n\nBlack has been checkmated" << std::endl;
			break;
		}
	
		move = std::string_view(result[2].begin() + 11, result[2].end());
		if (move[0] == 'a' && move[1] == '1' && move[2] == 'a' && move[3] == '1')
		{
			std::cout << "Draw" << std::endl;
		}

		std::from_chars(result[4].data() + 10, result[4].data() + result[4].length(), time);

		elapsed = std::chrono::nanoseconds(time);
		if (elapsed > blackClock + increment)
		{
			std::cout << "Black has run out of time" << std::endl;
			break;
		}

		blackClock -= elapsed;
		blackClock += increment;
		
		black.stdin() << "move " << move << std::endl;
		white.stdin() << "move " << move << std::endl;

		allMoves += move;
		allMoves += ' ';

		std::cout << "------BLACK------" << std::endl;
		std::cout << result[0] << std::endl;
		std::cout << "eval: " << eval << std::endl;
		std::cout << "move: " << move << std::endl;
		std::cout << "elapsed: " << elapsed.count() << std::endl;
	}
end:
	std::cout << "\n\n all moves: " << allMoves << std::endl;
	
	// std::cout << "PARENT received: " << str << std::endl;
	return 0;
}

// 
// Example of communication with a subprocess via stdin/stdout
// Author: Konstantin Tretyakov
// License: MIT
//

/*#include <ext/stdio_filebuf.h> // NB: Specific to libstdc++
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include <memory>
#include <exception>

// Wrapping pipe in a class makes sure they are closed when we leave scope
class cpipe {
private:
    int fd[2];
public:
    const inline int read_fd() const { return fd[0]; }
    const inline int write_fd() const { return fd[1]; }
    cpipe() { if (pipe(fd)) throw std::runtime_error("Failed to create pipe"); }
    void close() { ::close(fd[0]); ::close(fd[1]); }
    ~cpipe() { close(); }
};


//
// Usage:
//   spawn s(argv)
//   s.stdin << ...
//   s.stdout >> ...
//   s.send_eol()
//   s.wait()
//

class spawn {
private:
    cpipe write_pipe;
    cpipe read_pipe;
public:
    int child_pid = -1;
    std::unique_ptr<__gnu_cxx::stdio_filebuf<char> > write_buf = NULL; 
    std::unique_ptr<__gnu_cxx::stdio_filebuf<char> > read_buf = NULL;
    std::ostream stdin;
    std::istream stdout;
    
    spawn(const char* const argv[], bool with_path = false, const char* const envp[] = 0): stdin(NULL), stdout(NULL) {
        child_pid = fork();
        if (child_pid == -1) throw std::runtime_error("Failed to start child process"); 
        if (child_pid == 0) {   // In child process
            dup2(write_pipe.read_fd(), STDIN_FILENO);
            dup2(read_pipe.write_fd(), STDOUT_FILENO);
            write_pipe.close(); read_pipe.close();
            int result;
            if (with_path) {
                if (envp != 0) result = execvpe(argv[0], const_cast<char* const*>(argv), const_cast<char* const*>(envp));
                else result = execvp(argv[0], const_cast<char* const*>(argv));
            }
            else {
                if (envp != 0) result = execve(argv[0], const_cast<char* const*>(argv), const_cast<char* const*>(envp));
                else result = execv(argv[0], const_cast<char* const*>(argv));
            }
            if (result == -1) {
               // Note: no point writing to stdout here, it has been redirected
               std::cerr << "Error: Failed to launch program" << std::endl;
               exit(1);
            }
        }
        else {
            close(write_pipe.read_fd());
            close(read_pipe.write_fd());
            write_buf = std::unique_ptr<__gnu_cxx::stdio_filebuf<char> >(new __gnu_cxx::stdio_filebuf<char>(write_pipe.write_fd(), std::ios::out));
            read_buf = std::unique_ptr<__gnu_cxx::stdio_filebuf<char> >(new __gnu_cxx::stdio_filebuf<char>(read_pipe.read_fd(), std::ios::in));
            stdin.rdbuf(write_buf.get());
            stdout.rdbuf(read_buf.get());
        }
    }
    
    void send_eof() { write_buf->close(); }
    
    int wait() {
        int status;
        waitpid(child_pid, &status, 0);
        return status;
    }
};




// ---------------- Usage example -------------------- //
#include <string>
using std::string;
using std::getline;
using std::cout;
using std::endl;

int main() {
    const char* const argv[] = {"./main", (const char*)0};
    spawn cat(argv);
    cat.stdin << "eval 7i" << std::endl;
    string s;
    getline(cat.stdout, s);
    cout << "Read from program: '" << s << "'" << endl;
    cat.send_eof();
    cout << "Waiting to terminate..." << endl;
    cout << "Status: " << cat.wait() << endl;
    return 0;
}*/