#include <rclcpp/rclcpp.hpp>
#include "diffbot_msgs/srv/add_two_ints.hpp"
#include <memory>

using namespace std::placeholders;


class SimpleServiceServer : public rclcpp::Node
{
public:
    SimpleServiceServer() : Node("simple_service_server")
    {
        service_ = create_service<diffbot_msgs::srv::AddTwoInts>("add_two_ints", std::bind(&SimpleServiceServer::serviceCallback, this, _1, _2));
        RCLCPP_INFO(get_logger(), "Service add_two_ints Ready");    

    }

    void serviceCallback(const std::shared_ptr<diffbot_msgs::srv::AddTwoInts::Request> req,
        const std::shared_ptr<diffbot_msgs::srv::AddTwoInts::Response> res)
    {
        RCLCPP_INFO_STREAM(get_logger(), "New Request received a: " << req->a << " b: " << req->b);
        res->sum = req->a + req->b;
        RCLCPP_INFO_STREAM(get_logger(), "Returning sum: " << res->sum);
    }
private:
    rclcpp::Service<diffbot_msgs::srv::AddTwoInts>::SharedPtr service_;
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<SimpleServiceServer>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}