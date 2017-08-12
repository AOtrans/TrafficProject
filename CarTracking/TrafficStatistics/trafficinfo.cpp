#include "TrafficInfo.h"

Trainfo::Trainfo(){}

void Trainfo::excute(Detector *detector, const string& capture_name, const string& outFile){
    int count = 0;
    float squarelap = 0;
    std::vector<vector<float> > List;
    cv::Mat img;
    int frameflag = 0;

    VideoCapture capture;

    capture.open(capture_name);

    //cv::VideoWriter write;
    int w = static_cast<int>(capture.get(CV_CAP_PROP_FRAME_WIDTH));
    int h = static_cast<int>(capture.get(CV_CAP_PROP_FRAME_HEIGHT));
    cv::Size S(w, h);
    double r = capture.get(CV_CAP_PROP_FPS);
    //write.open(outFlie, CV_FOURCC('M','J','P','G'), r, S, true);
    cv::VideoWriter writer(outFile, CV_FOURCC('D', 'I', 'V', 'X'), 25.0, S, true);

    while (capture.read(img)){
        frameflag++;
        if(frameflag > 300){
            if(frameflag % 1000 == 0)
                List.clear();

            std::vector<vector<float> > results = detector->Detect(img);

            squarelap = congestion(results, img);

            results = box_transform(results);
            count += flow(results, List, img);

            for (int i = 0; i < results.size(); ++i) {
                const vector<float>& d = results[i];
                cv::Rect rect(d[1], d[2], (d[3] - d[1]), (d[4] - d[2]));
                cv::rectangle(img, rect, cvScalar(0, 255, 0), 2, 8, 0);
            }
            std::cout<<count<<std::endl;
            char text[5];
            snprintf(text, sizeof(text), "%d", count);
            cv::putText(img, text, Point(0, 150), FONT_HERSHEY_SIMPLEX, 2.5, Scalar(0, 0, 255), 3);
            char text_2[5];
            snprintf(text_2, sizeof(text), "%f", squarelap);
            cv::putText(img, text_2, Point(0, 300), FONT_HERSHEY_SIMPLEX, 2.5, Scalar(0, 0, 255), 3);
            cv::resize(img, img, cv::Size(2136, 1200));
            cv::imshow("result", img);
            writer.write(img);
            cv::waitKey(1);
        }
    }
    writer.release();
}

void Trainfo::CrossingDemo(Detector *detector, const string& capture_name,  const string& outFile, vector<vector<float> >rectboxes){

    int box_count = rectboxes.size();

    int count[box_count] = {0};
    int countflag[box_count] = {0};
    int repeatflag[box_count] = {0};
    cv::Rect rectbox[box_count];

    for(int i = 0; i < box_count; i++){
        rectbox[i].x = rectboxes[i][1];
        rectbox[i].y = rectboxes[i][2];
        rectbox[i].width = rectboxes[i][3] - rectboxes[i][1];
        rectbox[i].height = rectboxes[i][4] - rectboxes[i][2];
    }

    cv::Mat img;
    int frameflag = 0;

    VideoCapture capture;

    capture.open(capture_name);


    int w = static_cast<int>(capture.get(CV_CAP_PROP_FRAME_WIDTH));
    int h = static_cast<int>(capture.get(CV_CAP_PROP_FRAME_HEIGHT));
    cv::Size S(1000, 600);
    double r = capture.get(CV_CAP_PROP_FPS);
    //write.open(outFlie, CV_FOURCC('M','J','P','G'), r, S, true);
    cv::VideoWriter writer(outFile, CV_FOURCC('D', 'I', 'V', 'X'), 25.0, S, true);


    while (capture.read(img)){
        frameflag++;

        std::vector<vector<float> > results = detector->Detect(img);

        results = box_transform(results);
        //count += flow(results, List, img);

        for(int k = 0; k < box_count; k++){
            for (int i = 0; i < results.size(); ++i) {
                const vector<float>& d = results[i];
                cv::Rect rect(d[1], d[2], (d[3] - d[1]), (d[4] - d[2]));
                cv::rectangle(img, rect, cvScalar(0, 255, 0), 2, 8, 0);
                if(overlap(d, rectboxes[k]) > 0){
                    //overlap(d, rectboxtrans);
                    if(overlap(d, rectboxes[k]) >= 0.4 && countflag[k] == 0){
                        count[k]++;
                        countflag[k] = 1;
                        repeatflag[k] = 0;
                    }

                    if(overlap(d, rectboxes[k]) <= 0.4 && countflag[k] == 1){
                        ++repeatflag[k];
                        if(repeatflag[k] > 10)
                            countflag[k] = 0;
                    }
                }
            }
        }
        for(int i = 0; i < box_count; i++){
            cv::rectangle(img, rectbox[i], cvScalar(0, 0, 255), 2, 8, 0);
        }
        //std::cout<<count<<std::endl;
        for(int i = 0; i < box_count; i++){
            char text[5];
            snprintf(text, sizeof(text), "%d", count[i]);
            cv::putText(img, text, Point(rectboxes[i][1], rectboxes[i][2]), FONT_HERSHEY_SIMPLEX, 2.5, Scalar(255, 0, 0), 3);
        }
        cv::resize(img, img, cv::Size(1000, 600));
        cv::imshow("result", img);
        writer<<img;
        cv::waitKey(1);
    }
    writer.release();
}


float Trainfo::congestion(std::vector<vector<float> > results, cv::Mat img){
    float square = 0.0;
    for (int i = 0; i < results.size(); ++i) {
        const vector<float>& d = results[i];
        if (d[0] >= 0.5) {
            square += (d[3] - d[1]) * (d[4] - d[2]);
        }
    }
    square = square / (img.cols * img.rows);
    return square;
}

int Trainfo::flow(std::vector<vector<float> > results, std::vector<vector<float> > &List, cv::Mat img){
    int car = 0;

    if(results.size() == 0)
        if(List.size() != 0)
            List.clear();

    if(List.size() == 0){
        for (int i = 0; i < results.size(); ++i) {
            const vector<float>& d = results[i];
            List.push_back(d);
            car++;
        }
        return car;
    }

    for (int i = 0; i < results.size(); ++i) {
        const vector<float>& d = results[i];
        int flag = 0;
        for(int j = 0; j < List.size(); ++j) {
            if(overlap(d, List[j]) > 0.5){
                List[j] = d;
                flag = 1;
            }
        }
        if(flag == 0){
            List.push_back(d);
            car++;
        }
    }
    return car;
}

std::vector<vector<float> > Trainfo::box_transform(std::vector<vector<float> > box){
    std::vector<vector<float> > temp;
    for (int i = 0; i < box.size(); ++i) {
        const vector<float>& d = box[i];
        if (d[0] >= 0.9) {
            temp.push_back(d);
        }
    }
    return temp;
}

int Trainfo::car_count(std::vector<float> a, std::vector<vector<float> > b){
    int flag = 1;
    for (int i = 0; i < b.size(); ++i){
        if(overlap(b[i], a) > 0.5)
            flag = 0;
    }
    return flag;
}

float Trainfo::max_f(float a, float b){
    if(a > b)
        return a;
    else
        return b;
}

float Trainfo::min_f(float a, float b){
    if(a < b)
        return a;
    else
        return b;
}

float Trainfo::overlap(std::vector<float> a, std::vector<float> b){
    float x_min = min_f(a[1], b[1]);
    float y_min = min_f(a[2], b[2]);
    float x_max = max_f(a[3], b[3]);
    float y_max = max_f(a[4], b[4]);
    if (((x_max - x_min) >= (a[3] + b[3] - a[1] - b[1])) || ((y_max - y_min) >= (a[4] + b[4] - a[2] - b[2])))
        return 0.0;
    else{
        float lap = ((a[3] + b[3] - a[1] - b[1]) - (x_max - x_min)) * ((a[4] + b[4] - a[2] - b[2]) - (y_max - y_min));
        float square = (a[3] - a[1]) * (a[4] - a[2]) + (b[3] - b[1]) * (b[4] - b[2]);
        return (lap / (square - lap));
    }
}

float Trainfo::singleoverlap(std::vector<float> a, std::vector<float> b){
    float x_min = min_f(a[1], b[1]);
    float y_min = min_f(a[2], b[2]);
    float x_max = max_f(a[3], b[3]);
    float y_max = max_f(a[4], b[4]);
    if (((x_max - x_min) >= (a[3] + b[3] - a[1] - b[1])) || ((y_max - y_min) >= (a[4] + b[4] - a[2] - b[2])))
        return 0.0;
    else{
        float lap = ((a[3] + b[3] - a[1] - b[1]) - (x_max - x_min)) * ((a[4] + b[4] - a[2] - b[2]) - (y_max - y_min));
        float square = (a[3] - a[1]) * (a[4] - a[2]);
        return (lap / square);
    }
}
