cv::Vec3b processPixel(cv::Vec3b pixAbove, cv::Vec3b pixBelow) {
    // For multi-channel images (e.g., cv::Vec3b)
    cv::Vec3b diffPixel = cv::Vec3b(0, 0, 0);
    for (int i = 0; i < pixAbove.channels; ++i) {
        int tempValueA = static_cast<int>(pixAbove[i]);
        int tempValueB = static_cast<int>(pixBelow[i]);
        int dif = std::abs(tempValueA - tempValueB) * 10;
        if(dif > 255) dif = 255;
        if(dif < 0) dif = 0;
        diffPixel[i] = static_cast<uchar>(dif);
    }
    return diffPixel;
}

cv::Mat getDifferenceImage(cv::Mat image) {    
    //get difference image
    cv::Mat diffImage(image.rows - 1, image.cols, CV_8UC3);
    for (int i = 1; i < image.rows - 1; i++) {
        for (int j = 0; j < image.cols; j++) {
            cv::Vec3b pixelAbove = image.at<cv::Vec3b>(i - 1, j);
            cv::Vec3b pixelBelow = image.at<cv::Vec3b>(i, j);
            cv::Vec3b diffPixel = processPixel(pixelAbove,pixelBelow);
            diffImage.at<cv::Vec3b>(i - 1, j) = diffPixel;
        }
    }
    return diffImage;
}