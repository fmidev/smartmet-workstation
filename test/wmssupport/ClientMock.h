#pragma once
#include "gmock/gmock.h"
#include "cpprest/http_client.h"

class ClientMock
{
public:
    std::string serverAddress_;

    ClientMock()
        : serverAddress_()
    {
    }

    ClientMock(const ClientMock &other)
        :serverAddress_(other.serverAddress_)
    {
    }

    ClientMock(std::string serverAddress)
    {
        serverAddress_ = serverAddress;
    }

    ClientMock& operator=(ClientMock other)
    {
        swap(*this, other);
        
        return *this;
    }

    ClientMock(ClientMock &&other)
        :serverAddress_()
    {
        swap(*this, other);
    }

    friend void swap(ClientMock &client1, ClientMock &client2)
    {
        using std::swap;
        swap(client1.serverAddress_, client2.serverAddress_);
    }

    MOCK_METHOD3(request, pplx::task< web::http::http_response>(const web::http::method &mtd, const utility::string_t &path_query_fragment, const pplx::cancellation_token &token));
};
