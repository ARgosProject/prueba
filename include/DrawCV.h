#ifndef _argosToolkit_DrawUtils_H_
#define _argosToolkit_DrawUtils_H_

#include <opencv2/opencv.hpp>
#include <cstdio>
#include <iostream>
#include "Paper.h"
#include "CameraProjectorSystem.h"

namespace argosServer{
  
  /** 
   * A set of functions to draw in opencv images
   */
  class DrawCV{
    
  public:
    /********************************************************************************************
     *  SCREEN DISPLAY
     ********************************************************************************************/
    
    /** 
     * Draws 3D coordinate axis in center of paper
     */
    static void draw3DAxis(cv::Mat& image, Paper& p, CameraProjectorSystem& cameraProjector);

    static void draw3DAxisInPoint(cv::Mat& image, Paper& paper, CameraProjectorSystem& cameraProjector, cv::Point2f& finger);

    /** 
     * Draws 3D cube in center of paper
     */
    static void draw3DCube(cv::Mat& image, Paper& p, CameraProjectorSystem& cameraProjector);
    /** 
     *  Draws 3D cube overlaping the paper
     */
    static void draw3DPaper(cv::Mat& image, Paper& paper, CameraProjectorSystem& cameraProjector);
    
    /********************************************************************************************
     * PROJECTOR  DISPLAY
     ********************************************************************************************/
    
    /** 
     * Draws the invoice projected
     */
    static void projectPaper(cv::Mat& image, Paper& paper, CameraProjectorSystem& cameraProjector, cv::Mat& video,  cv::Mat& out);
    /** 
     * Draws the invoice projected (version 2)
     */
    static void projectPaper0(cv::Mat& image, Paper& paper, CameraProjectorSystem& cameraProjector, int index);
     /** 
     * Draws the invoice projected in screen
     */
    static void cameraPaper(cv::Mat& image, Paper& paper, CameraProjectorSystem& cameraProjector, const string& text);
    /** 
     * Draws projection limits 
     */
    static void drawBoardMarks(cv::Mat& image, int thickness, int size);
    /** 
     * Draws the invoice projected (version 3)
     */
    static void drawInvoice(cv::Mat& image, Paper& paper, CameraProjectorSystem& cameraProjector);

  private:
    /** 
     * Draws contour of paper
     */
    static void drawContour(cv::Mat &in, vector<cv::Point2f>& contour, cv::Scalar color);
  };
}
#endif

