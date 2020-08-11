#include <parameterised_tests.h>
#include <opencv_api_calls.h>

TEST_P(OpenCV_API_ParamTest, TrackerCreationTest) {

	int tracker_index = GetParam();
	ASSERT_TRUE(OPENCV_API_CALLS::check_ObjectTrackerInitialisation(tracker_index));

}

//INSTANTIATE_TEST_CASE_P(InstantiationName, MyStringTest,::testing::Values("meek", "geek", "freek"));
																		  // account_state{ initial_balance, withdraw_amount, final_balance, success }
INSTANTIATE_TEST_CASE_P(CheckTrackerCreationSuccess, OpenCV_API_ParamTest, testing::Values( 0, 1, 2, 3, 4, 5, 6, 7));
