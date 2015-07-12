#include "server.h"
#include "threadpool.h"
using std::cout;
using std::cin;
using std::endl;
using boost::asio::ip::tcp;

class server
{
public:
    server(short port, int thread_cnt)
        : io_service_pool_(thread_cnt)
        , acceptor_(io_service_pool_.get_io_service(), tcp::endpoint(tcp::v4(), port))
    {
        session* new_session = new session(io_service_pool_.get_io_service());
        acceptor_.async_accept(new_session->socket(),
            boost::bind(&server::handle_accept, this, new_session, boost::asio::placeholders::error));
    }

    void handle_accept(session* new_session, const boost::system::error_code& error)
    {
        if (!error)
        {
            new_session->start();
        }
        else
        {
            delete new_session;
        }

        new_session = new session(io_service_pool_.get_io_service());
        acceptor_.async_accept(new_session->socket(),
            boost::bind(&server::handle_accept, this, new_session, boost::asio::placeholders::error));
    }

    void run()
    {
        io_service_pool_.start();
        io_service_pool_.join();
    }

private:

    io_service_pool io_service_pool_;
    tcp::acceptor acceptor_;
};

int main()
{

    //boost
    server s(6002, 1);
    s.run();

    while(true)
    {
        sleep(1000);


     }

    return 0;
}
