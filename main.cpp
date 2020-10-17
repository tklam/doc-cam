#include <opencv2/opencv.hpp>
#include <vector>
#include <climits>
#include <cmath>

#include "img_source.h"

#define DEBUG

#ifdef DEBUG
#define debug_code(x) (x)
#else
#define debug_code(x) ()
#endif

// sort counter clockwise with reference to OpenCV's coordinate system
template <typename P>
struct SortCounterClockwise {
    P centre;
    bool operator() (P const & a, P const & b) {
        if (a.x - centre.x >= 0 && b.x - centre.x < 0) {
            return true;
        }
        if (a.x - centre.x < 0 && b.x - centre.x >= 0) {
            return false;
        }
        if (a.x - centre.x == 0 && b.x - centre.x == 0) {
            if (a.y - centre.y >= 0 || b.y - centre.y >= 0) {
                return a.y > b.y;
            }
            return b.y > a.y;
        }

        // compute the cross product of vectors (centre -> a) x (centre -> b)
        int det = (a.x - centre.x) * (b.y - centre.y) - (b.x - centre.x) * (a.y - centre.y);
        if (det < 0) {
            return true;
        }
        if (det > 0) {
            return false;
        }

        // points a and b are on the same line from the centre
        // check which point is closer to the centre
        int d1 = (a.x - centre.x) * (a.x - centre.x) + (a.y - centre.y) * (a.y - centre.y);
        int d2 = (b.x - centre.x) * (b.x - centre.x) + (b.y - centre.y) * (b.y - centre.y);
        return d1 > d2;
        return false;
    }
};

void make_proper_contour(std::vector<cv::Point> & points) {
    // sort counter clockwise
    SortCounterClockwise<cv::Point> sort_clockwise;
    // find the centre
    for (auto const i : points) {
        sort_clockwise.centre.x += i.x;
        sort_clockwise.centre.y += i.y;
    }
    sort_clockwise.centre.x = sort_clockwise.centre.x / points.size();
    sort_clockwise.centre.y = sort_clockwise.centre.y / points.size();
    std::sort(points.begin(), points.end(), sort_clockwise);

//    // counter clockwise
//    std::reverse(points.begin(), points.end());

    // sort such that the frst point is the nearest to (0,0)
    int min_dist = INT_MAX;
    size_t min_index = 0;
    for (size_t i=0; i<points.size(); ++i) {
        int dist = (int)sqrt(points[i].x*points[i].x + points[i].y*points[i].y);
        if (dist < min_dist) {
            min_dist = dist;
            min_index = i;
        }
    }

    // make the point with the lowest Y coordinate the first element
    for (size_t i=0; i<min_index; ++i) {
        points.emplace_back(points[i]);
    }
    points.erase(points.begin(), points.begin()+min_index);
}

std::vector<cv::Point> const & calc_largest_contour(std::vector<std::vector<cv::Point>> &contours) {
    std::vector<cv::Point> const * largest;
    int size = 0;
    for(auto const & c : contours) {
        int area = cv::contourArea(c);
        if (area > size) {
            largest = &c;
            size = area;
        }
    }
    return *largest;
}

void display_img(cv::Mat disp) {
    do {
        cv::imshow("Current image", disp);
        if (cv::waitKey(30) == 27) {
            return;
        }
    } while (true);
}

void preprocess_doc_identification(cv::Mat const & img, cv::Mat & result) {
    using namespace cv;
    using namespace std;

    // make it gray-scaled
    Mat gray;
    cvtColor(img, gray, COLOR_BGR2GRAY);

    // blur image
    Mat blurred;
    bilateralFilter(gray, blurred, 5, 75, 75);

    // apply canny edge detection to image
    Canny(blurred, result, 10, 150);

    debug_code(display_img(result));
}

bool identify_doc_by_contour(cv::Mat const & img, std::vector<cv::Point> & best_contour) {
    using namespace cv;
    using namespace std;

    //find the contours
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(img, contours, noArray(), RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    bool const empty_countours = contours.empty();
    debug_code(printf("Number of contours: %d\n", contours.size()));

    if (!empty_countours) {
        auto const & max_contour = calc_largest_contour(contours);

        debug_code({
            Scalar color = Scalar(128, 128, 128, 128);
            vector<vector<Point>> draw;
            draw.emplace_back(max_contour);
            Mat drawing = img.clone();
            drawContours( drawing, draw, 0, color );
            display_img(drawing);
            draw.clear();
            vector<Point> max_convex_hull;
            convexHull(max_contour, max_convex_hull);
            draw.emplace_back(max_convex_hull);
            drawContours( drawing, draw, 0, color );
            display_img(drawing);
        });

        // some filters to get rid of the potentially inappropriate contour
        
        // assume the area of the max contour > 25% of the pic area
        float const contour_area = contourArea(max_contour);
        float const img_area = img.rows * img.cols;
        if (contour_area < img_area * 0.25f) {
            debug_code(
                printf("contour area: %f image area: %f\n", contour_area, img_area)
            );
            return false;
        }

        best_contour = max_contour;

        return true;
    }

    return false;
}

bool warp_contour_perspective(cv::Mat const & original_img, std::vector<cv::Point> const & best_contour, int const dest_width) {
    // XXX assume best contour has >=4 points
    
    using namespace cv;
    using namespace std;
    vector<Point> hull;
    convexHull(best_contour, hull);

    // find a rotated rectangle of the minimum area enclosing the input 2D point set.
    RotatedRect const rect = minAreaRect(hull);

    // approximate the best contour with only 4 points
    float const ratio = rect.size.height / rect.size.width;
    debug_code(
        printf("detected rect of convex hull's ratio: %f\n", ratio)
    );

    vector<Point> approx_contour_int_pts;
    int epsilon = 0;
    while (approx_contour_int_pts.size() != 4) {
        epsilon++;

        approxPolyDP(hull, approx_contour_int_pts, epsilon, true);
        if (epsilon == 100) {
            printf("Failed to simplify the comtour, pts: %d\n", approx_contour_int_pts.size());
            // cannot simplify the contour into 4 points
            return false;
        }
    }

    // make approx_contour_.pts into a proper contour:
    make_proper_contour(approx_contour_int_pts);
    vector<Point2f> approx_contour_pts;
    for (auto const i : approx_contour_int_pts) {
        approx_contour_pts.emplace_back(std::move(Point2f(i.x, i.y)));
        debug_code(
            printf("approx contour: (%d, %d)\n", i.x, i.y)
        );
    }

    // define the destination coordinates
    vector<Point2f> target_coors;
    int const dest_height = dest_width * ratio;
    target_coors.emplace_back(Point2f(0, 0));
    target_coors.emplace_back(Point2f(0, dest_height));
    target_coors.emplace_back(Point2f(dest_width, dest_height));
    target_coors.emplace_back(Point2f(dest_width, 0));

    // transfrom approx contour points into target_coors (change the perspective)
    Mat const trans_mat = getPerspectiveTransform(approx_contour_pts, target_coors);
    Mat result = Mat(Size(dest_width, dest_height), CV_64F);
    warpPerspective(original_img, result, trans_mat, result.size());
    debug_code(display_img(result));

    return true;
}

int main(int argc, char** argv) {
    using namespace cv;
    using namespace std;

    //---------------------- capture an image
    FileImageSource img_source;
    img_source.filename = string(argv[1]);
    Mat const img = img_source.get_image();

    //---------------------- pre-process for document identification
    Mat preprocessed_frame;
    preprocess_doc_identification(img, preprocessed_frame);

    //---------------------- identify the docutment
    vector<Point> best_contour;
    bool const can_find_contour = identify_doc_by_contour(preprocessed_frame, best_contour);

    bool const is_success_contour = can_find_contour;
    if (is_success_contour) {
        warp_contour_perspective(img, best_contour, 1000);
    }

    return 0;
}

