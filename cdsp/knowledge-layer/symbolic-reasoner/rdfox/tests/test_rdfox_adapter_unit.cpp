#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../src/rdfox-adapter.h"

class MockRDFoxAdapter : public RDFoxAdapter {
   public:
    MockRDFoxAdapter() : RDFoxAdapter("localhost", "8080", "test_auth", "test_ds") {}

    MOCK_METHOD6(sendRequest, bool(http::verb, const std::string&, const std::string&,
                                   const std::string&, const std::string&, std::string&));
};

// Test checking nonexistent RDFox datastore.
TEST(RDFoxAdapterTest, CheckNonexistentDatastore) {
    MockRDFoxAdapter mock_adapter;

    // Mock the sendRequest for checking nonexistent datastore.
    EXPECT_CALL(mock_adapter, sendRequest(http::verb::get, "/datastores", "", "",
                                          "text/csv; charset=UTF-8", testing::_))
        .WillOnce(testing::DoAll(testing::SetArgReferee<5>(""), testing::Return(false)));

    EXPECT_FALSE(mock_adapter.checkDataStore());
}

// Test checking existent RDFox datastore.
TEST(RDFoxAdapterTest, CheckExistentDatastore) {
    MockRDFoxAdapter mock_adapter;

    // Mock the sendRequest for checking existent datastore.
    EXPECT_CALL(mock_adapter, sendRequest(http::verb::get, "/datastores", "", "",
                                          "text/csv; charset=UTF-8", testing::_))
        .WillOnce(testing::DoAll(testing::SetArgReferee<5>("test_ds"), testing::Return(true)));

    EXPECT_TRUE(mock_adapter.checkDataStore());
}

// Test the initialization of the RDFoxAdapter and creation of the datastore.
TEST(RDFoxAdapterTest, InitializationCreatesDatastore) {
    MockRDFoxAdapter mock_adapter;

    // Mock the sendRequest for checking datastore does not exists.
    EXPECT_CALL(mock_adapter, sendRequest(http::verb::get, "/datastores", "", "",
                                          "text/csv; charset=UTF-8", testing::_))
        .WillOnce(testing::DoAll(testing::SetArgReferee<5>(""), testing::Return(false)));

    // Mock the sendRequest for creating the datastore.
    EXPECT_CALL(mock_adapter, sendRequest(http::verb::post, "/datastores/test_ds", "",
                                          "application/json", "", testing::_))
        .WillOnce(testing::Return(true));

    EXPECT_NO_THROW(mock_adapter.initialize());
}

// Test the initialization of the RDFoxAdapter when datastore already exists.
TEST(RDFoxAdapterTest, InitializationWithExistingDatastore) {
    MockRDFoxAdapter mock_adapter;

    // Mock the sendRequest for checking datastore exists.
    EXPECT_CALL(mock_adapter, sendRequest(http::verb::get, "/datastores", "", "",
                                          "text/csv; charset=UTF-8", testing::_))
        .WillOnce(testing::DoAll(testing::SetArgReferee<5>("test_ds"), testing::Return(true)));

    // Ensure that the second call for creating the datastore is not made.
    EXPECT_CALL(mock_adapter, sendRequest(http::verb::post, "/datastores/test_ds", "",
                                          "application/json", "", testing::_))
        .Times(0);

    EXPECT_NO_THROW(mock_adapter.initialize());
}

// Test loading data into the RDFox datastore.
TEST(RDFoxAdapterTest, LoadDataSuccess) {
    MockRDFoxAdapter mock_adapter;
    std::string ttl_data = "@prefix : <http://example.org/> . :test a :Entity .";

    // Mock the sendRequest for loading data.
    EXPECT_CALL(mock_adapter, sendRequest(http::verb::post, "/datastores/test_ds/content", ttl_data,
                                          "text/turtle", "", testing::_))
        .WillOnce(testing::Return(true));

    EXPECT_TRUE(mock_adapter.loadData(ttl_data));
}

// Test querying data from the RDFox datastore.
TEST(RDFoxAdapterTest, QueryDataSuccess) {
    MockRDFoxAdapter mock_adapter;
    std::string sparql_query = "SELECT ?s WHERE { ?s ?p ?o . }";
    std::string mock_response = "<http://example.org/test>";

    // Mock the sendRequest for querying data.
    EXPECT_CALL(mock_adapter, sendRequest(http::verb::post, "/datastores/test_ds/sparql",
                                          sparql_query, "application/sparql-query", "", testing::_))
        .WillOnce(testing::DoAll(testing::SetArgReferee<5>(mock_response), testing::Return(true)));

    EXPECT_EQ(mock_adapter.queryData(sparql_query), mock_response);
}

// Test deleting the RDFox datastore.
TEST(RDFoxAdapterTest, DeleteDataStoreSuccess) {
    MockRDFoxAdapter mock_adapter;

    // Mock the sendRequest for checking the datastore existence.
    EXPECT_CALL(mock_adapter, sendRequest(http::verb::get, "/datastores", "", "",
                                          "text/csv; charset=UTF-8", testing::_))
        .WillOnce(testing::DoAll(testing::SetArgReferee<5>("test_ds"), testing::Return(true)));

    // Mock the sendRequest for deleting the datastore.
    EXPECT_CALL(mock_adapter,
                sendRequest(http::verb::delete_, "/datastores/test_ds", "", "", "", testing::_))
        .WillOnce(testing::Return(true));

    EXPECT_TRUE(mock_adapter.deleteDataStore());
}

// Test any error deleting the RDFox datastore that does not exists.
TEST(RDFoxAdapterTest, DeleteNonexistentDataStoreSuccess) {
    MockRDFoxAdapter mock_adapter;

    // Mock the sendRequest for checking the datastore existence.
    EXPECT_CALL(mock_adapter, sendRequest(http::verb::get, "/datastores", "", "",
                                          "text/csv; charset=UTF-8", testing::_))
        .WillOnce(testing::DoAll(testing::SetArgReferee<5>(""), testing::Return(false)));

    // Ensure that the second call for deleting the datastore is not made.
    EXPECT_CALL(mock_adapter,
                sendRequest(http::verb::delete_, "/datastores/test_ds", "", "", "", testing::_))
        .Times(0);

    EXPECT_TRUE(mock_adapter.deleteDataStore());
}

// Test handling a failed request during datastore creation.
TEST(RDFoxAdapterTest, FailedToCreateDataStore) {
    MockRDFoxAdapter mock_adapter;

    // Mock the sendRequest for checking the datastore existence.
    EXPECT_CALL(mock_adapter, sendRequest(http::verb::get, "/datastores", "", "",
                                          "text/csv; charset=UTF-8", testing::_))
        .WillOnce(testing::DoAll(testing::SetArgReferee<5>(""), testing::Return(true)));

    // Mock the sendRequest for creating the datastore failure.
    EXPECT_CALL(mock_adapter, sendRequest(http::verb::post, "/datastores/test_ds", "",
                                          "application/json", "", testing::_))
        .WillOnce(testing::Return(false));

    // Expect a runtime error when the adapter tries to create the datastore.
    EXPECT_THROW(mock_adapter.initialize(), std::runtime_error);
}

// Test loading data when the request fails.
TEST(RDFoxAdapterTest, LoadDataFailure) {
    MockRDFoxAdapter mock_adapter;
    std::string ttl_data = "@prefix : <http://example.org/> . :test a :Entity .";

    // Mock the sendRequest for loading data failure.
    EXPECT_CALL(mock_adapter, sendRequest(http::verb::post, "/datastores/test_ds/content", ttl_data,
                                          "text/turtle", "", testing::_))
        .WillOnce(testing::Return(false));

    EXPECT_FALSE(mock_adapter.loadData(ttl_data));
}

// Test querying data when the request fails.
TEST(RDFoxAdapterTest, QueryDataFailure) {
    MockRDFoxAdapter mock_adapter;
    std::string sparql_query = "SELECT ?s WHERE { ?s ?p ?o . }";

    // Mock the sendRequest for querying data failure.
    EXPECT_CALL(mock_adapter, sendRequest(http::verb::post, "/datastores/test_ds/sparql",
                                          sparql_query, "application/sparql-query", "", testing::_))
        .WillOnce(testing::Return(false));

    // Call queryData and expect it to return an empty string.
    EXPECT_EQ(mock_adapter.queryData(sparql_query), "");
}

// Test deleting the RDFox datastore when delete request fails.
TEST(RDFoxAdapterTest, DeleteDataStoreFailure) {
    MockRDFoxAdapter mock_adapter;

    // Mock the sendRequest for checking the datastore existence.
    EXPECT_CALL(mock_adapter, sendRequest(http::verb::get, "/datastores", "", "",
                                          "text/csv; charset=UTF-8", testing::_))
        .WillOnce(testing::DoAll(testing::SetArgReferee<5>("test_ds"), testing::Return(true)));

    // Mock the sendRequest for deleting the datastore failure.
    EXPECT_CALL(mock_adapter,
                sendRequest(http::verb::delete_, "/datastores/test_ds", "", "", "", testing::_))
        .WillOnce(testing::Return(false));

    EXPECT_FALSE(mock_adapter.deleteDataStore());
}