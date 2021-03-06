#ifndef VISP_TRACKER_TRACKER_VIEWER_HH
# define VISP_TRACKER_TRACKER_VIEWER_HH
# include <boost/filesystem/path.hpp>
# include <boost/optional.hpp>

# include <geometry_msgs/PoseWithCovarianceStamped.h>

# include <image_proc/advertisement_checker.h>

# include <image_transport/image_transport.h>
# include <image_transport/subscriber_filter.h>

# include <message_filters/subscriber.h>
# include <message_filters/sync_policies/approximate_time.h>
# include <message_filters/synchronizer.h>

# include <sensor_msgs/Image.h>
# include <sensor_msgs/CameraInfo.h>

# include <visp_tracker/MovingEdgeSites.h>

# include <visp/vpCameraParameters.h>
# include <visp/vpImage.h>
# include <visp/vpMbEdgeTracker.h>

namespace visp_tracker
{
  /// \brief Monitors the tracking result provided by the tracking node.
  class TrackerViewer
  {
  public:
    /// \brief ViSP image type
    typedef vpImage<unsigned char> image_t;

    /// \brief Synchronization policy
    ///
    /// This is used to make sure that the image, the object position
    /// and the moving edge sites are synchronized. This may not be the
    /// case as these informations are published on different topics.
    /// The approximate time allows light differences in timestamps
    /// which are not critical as this is only a viewer.
    typedef message_filters::sync_policies::ApproximateTime<
      sensor_msgs::Image, sensor_msgs::CameraInfo,
      geometry_msgs::PoseWithCovarianceStamped,
      visp_tracker::MovingEdgeSites
      > syncPolicy_t;

    /// \brief Constructor.
    TrackerViewer(ros::NodeHandle& nh,
		  ros::NodeHandle& privateNh,
		  volatile bool& exiting,
		  unsigned queueSize = 5u);

    /// \brief Display camera image, tracked object position and moving
    /// edge sites.
    void spin();
  protected:
    /// \brief Initialize the tracker.
    void initializeTracker();

    /// \brief Make sure the topics we subscribe already exist.
    void checkInputs();

    /// \brief Hang until the first image is received.
    void waitForImage();

    /// \brief Callback used to received synchronized data.
    void
    callback
    (const sensor_msgs::ImageConstPtr& imageConst,
     const sensor_msgs::CameraInfoConstPtr& infoConst,
     const geometry_msgs::PoseWithCovarianceStamped::ConstPtr& trackingResult,
     const visp_tracker::MovingEdgeSites::ConstPtr& sitesConst);

    void timerCallback();

    /// \brief Display moving edge sites.
    void displayMovingEdgeSites();

  private:
    bool exiting ()
    {
      return exiting_ || !ros::ok();
    }

    volatile bool& exiting_;

    /// \brief Queue size for all subscribers.
    unsigned queueSize_;

    ros::NodeHandle& nodeHandle_;
    ros::NodeHandle& nodeHandlePrivate_;

    /// \brief Image transport used to receive images.
    image_transport::ImageTransport imageTransport_;


    /// \name Topics and services strings.
    /// \{

    /// \brief Full topic name for rectified image.
    std::string rectifiedImageTopic_;
    /// \brief Full topic name for camera information.
    std::string cameraInfoTopic_;

    /// \}


    /// \brief VRML model path.
    boost::filesystem::path vrmlPath_;

    /// \brief Helper used to check that subscribed topics exist.
    image_proc::AdvertisementChecker checkInputs_;

    /// \brief ViSP edge tracker.
    vpMbEdgeTracker tracker_;
    /// \brief ViSP camera parameters.
    vpCameraParameters cameraParameters_;
    /// \brief ViSP image.
    image_t image_;

    /// \brief Shared pointer to latest received camera information.
    sensor_msgs::CameraInfoConstPtr info_;
    /// \brief Last tracked object position, set to none if tracking failed.
    boost::optional<vpHomogeneousMatrix> cMo_;
    /// \brief Shared pointer to latest received moving edge sites.
    visp_tracker::MovingEdgeSites::ConstPtr sites_;

    /// \name Subscribers and synchronizer.
    /// \{
    /// \brief Subscriber to image topic.
    image_transport::SubscriberFilter imageSubscriber_;
    /// \brief Subscriber to camera information topic.
    message_filters::Subscriber<sensor_msgs::CameraInfo> cameraInfoSubscriber_;
    /// \brief Subscriber to tracking result topic.
    message_filters::Subscriber<geometry_msgs::PoseWithCovarianceStamped>
    trackingResultSubscriber_;
    /// \brief Subscriber to moving edge sites topics..
    message_filters::Subscriber<visp_tracker::MovingEdgeSites>
    movingEdgeSitesSubscriber_;

    /// \brief Synchronizer with approximate time policy.
    message_filters::Synchronizer<syncPolicy_t> synchronizer_;
    ///}

    /// \name Synchronization check
    /// \{
    ros::WallTimer timer_;
    unsigned countAll_;
    unsigned countImages_;
    unsigned countCameraInfo_;
    unsigned countTrackingResult_;
    unsigned countMovingEdgeSites_;
    ///}
  };
} // end of namespace visp_tracker

#endif //! VISP_TRACKER_TRACKER_VIEWER_HH
