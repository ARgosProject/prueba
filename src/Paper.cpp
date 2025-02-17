#define _USE_MATH_DEFINES
#include "Paper.h"
#include <math.h>
#include <cstdio>

namespace argosServer{

  /**
   * Constructor
   */
  Paper::Paper(){
    id=0;
    paperSize=cv::Size(-1,-1);
    rotVec.create(3,1,CV_32FC1);
    transVec.create(3,1,CV_32FC1);
    for (int i=0; i<3; i++)
      transVec.at<float>(i,0) = rotVec.at<float>(i,0)= -999999;
  }

  Paper::Paper(cv::Size size){
    id=0;
    paperSize = size;
    rotVec.create(3,1,CV_32FC1);
    transVec.create(3,1,CV_32FC1);
    for (int i=0; i<3; i++)
      transVec.at<float>(i,0) = rotVec.at<float>(i,0)= -999999;
  }


  Paper::Paper(const Paper &P): vector<cv::Point2f>(P){
    id=0;
    P.rotVec.copyTo(rotVec);
    P.transVec.copyTo(transVec);
    paperSize.width = P.paperSize.width;
    paperSize.height = P.paperSize.height;
  }

  /**
   *
   */
  Paper::Paper(const vector<cv::Point2f> &corners): vector<cv::Point2f>(corners){
    id=0;
    paperSize = cv::Size(-1,-1);
    rotVec.create(3,1,CV_32FC1);
    transVec.create(3,1,CV_32FC1);
    for (int i=0;i<3;i++)
      transVec.at<float>(i,0)=rotVec.at<float>(i,0)=-999999;
  }
  /**
   *
   */
  void Paper::glGetModelViewMatrix(float modelview_matrix[16]) throw(cv::Exception){
    //check if paremeters are valid
    bool invalid=false;

    for (int i=0;i<3 && !invalid ;i++){
      if (transVec.at<float>(i,0) != -999999) invalid |= false;
      if (rotVec.at<float>(i,0) != -999999) invalid |= false;
    }

    if (invalid) throw cv::Exception(9003,"extrinsic parameters are not set","Paper::getModelViewMatrix",__FILE__,__LINE__);

    cv::Mat rot3x3(3,3,CV_32FC1);
    cv::Mat Jacob;
    cv::Rodrigues(rotVec, rot3x3, Jacob);

    float para[3][4];
    for (int i=0; i<3; i++)
      for (int j=0; j<3; j++)
        para[i][j] = rot3x3.at<float>(i,j);

    //now, add the translation
    para[0][3] = transVec.at<float>(0,0);
    para[1][3] = transVec.at<float>(1,0);
    para[2][3] = transVec.at<float>(2,0);
    float scale=1.0;

    modelview_matrix[0 + 0*4] = para[0][0];
    // R1C2
    modelview_matrix[0 + 1*4] = para[0][1];
    modelview_matrix[0 + 2*4] = para[0][2];
    modelview_matrix[0 + 3*4] = para[0][3];
    // R2
    modelview_matrix[1 + 0*4] = para[1][0];
    modelview_matrix[1 + 1*4] = para[1][1];
    modelview_matrix[1 + 2*4] = para[1][2];
    modelview_matrix[1 + 3*4] = para[1][3];
    // R3
    modelview_matrix[2 + 0*4] = -para[2][0];
    modelview_matrix[2 + 1*4] = -para[2][1];
    modelview_matrix[2 + 2*4] = -para[2][2];
    modelview_matrix[2 + 3*4] = -para[2][3];
    modelview_matrix[3 + 0*4] = 0.0;
    modelview_matrix[3 + 1*4] = 0.0;
    modelview_matrix[3 + 2*4] = 0.0;
    modelview_matrix[3 + 3*4] = 1.0;

    if (scale != 0.0){
      modelview_matrix[12] *= scale;
      modelview_matrix[13] *= scale;
      modelview_matrix[14] *= scale;
    }
  }

  void Paper::draw(Mat &in, Scalar color, int lineWidth ,bool writeId) const{
    if (size()!= 4) return;
    cv::line(in,(*this)[0],(*this)[1],color,lineWidth,CV_AA);
    cv::line(in,(*this)[1],(*this)[2],color,lineWidth,CV_AA);
    cv::line(in,(*this)[2],(*this)[3],color,lineWidth,CV_AA);
    cv::line(in,(*this)[3],(*this)[0],color,lineWidth,CV_AA);
    cv::rectangle( in,(*this)[0]-Point2f(2,2),(*this)[0]+Point2f(2,2),Scalar(0,0,255,255),lineWidth,CV_AA);
    cv::rectangle( in,(*this)[1]-Point2f(2,2),(*this)[1]+Point2f(2,2),Scalar(0,255,0,255),lineWidth,CV_AA);
    cv::rectangle( in,(*this)[2]-Point2f(2,2),(*this)[2]+Point2f(2,2),Scalar(255,0,0,255),lineWidth,CV_AA);
    if (writeId) {
      char cad[100];
      sprintf(cad,"paper");
      //determine the centroid
      Point cent(0,0);
      for (int i=0; i<4; i++){
        cent.x+=(*this)[i].x;
        cent.y+=(*this)[i].y;
      }
      cent.x/= 4.;
      cent.y/= 4.;
      putText(in, cad, cent,FONT_HERSHEY_SIMPLEX, 0.5,  Scalar(255-color[0],255-color[1],255-color[2],255),2);
    }
  }

  void Paper::calculateExtrinsics(cv::Size paperSizeMeters, CameraProjectorSystem& cameraProjector, bool setYPerperdicular, bool screenExtrinsics) throw(cv::Exception){
    if (!cameraProjector.isValid())
      throw cv::Exception(9004,"!cameraProjector.isValid(): invalid camera or projector parameters. It is not possible to calculate extrinsics","calculateExtrinsics",__FILE__,__LINE__);

    if(screenExtrinsics)
      calculateScreenExtrinsics(paperSizeMeters, cameraProjector.getCamera(), setYPerperdicular);
    else
      calculateProjectorExtrinsics(paperSizeMeters, cameraProjector, setYPerperdicular);
  }

  void Paper::calculateProjectorExtrinsics(cv::Size paperSizeMeters,  CameraProjectorSystem& cameraProjector, bool setYPerperdicular) throw(cv::Exception){
    if (!isValid())
      throw cv::Exception(9004,"!isValid(): invalid paper. It is not possible to calculate extrinsics","calculateExtrinsics",__FILE__,__LINE__);
    if (paperSizeMeters.width <= 0 || paperSizeMeters.height <= 0)
      throw cv::Exception(9004,"paperSize <=0: invalid paper size","calculateExtrinsics",__FILE__,__LINE__);
    if (!cameraProjector.isValid())
      throw cv::Exception(9004,"invalid camera parameters","calculateExtrinsics",__FILE__,__LINE__);

    CameraModel& camera = cameraProjector.getCamera();
    //CameraModel& projector  = cameraProjector.getProjector();

    float halfWidthSize  =  paperSizeMeters.width/2.;
    float halfHeightSize =  paperSizeMeters.height/2.;

    double  distance_0_1 = cv::norm( (*this)[0] - (*this)[1] );
    double  distance_1_2 = cv::norm( (*this)[1] - (*this)[2] );
    double  distance_2_3 = cv::norm( (*this)[2] - (*this)[3] );
    double  distance_3_0 = cv::norm( (*this)[3] - (*this)[0] );


    //cout << "distance_0_1: "<<  distance_0_1 << endl;
    //cout << "distance_1_2: "<<  distance_1_2 << endl;
    //cout << "distance_2_3: "<<  distance_2_3 << endl;
    //cout << "distance_3_0: "<<  distance_3_0 << endl;

    //cout << "distance_01-23: "<<  (distance_0_1 + distance_2_3) << endl;
    //cout << "distance_12-30: "<<  (distance_1_2 + distance_3_0) << endl;

    cv::Mat ObjPoints(4,3,CV_32FC1);

    ObjPoints.at<float>(0,0)=-halfWidthSize;
    ObjPoints.at<float>(0,1)=-halfHeightSize;
    ObjPoints.at<float>(0,2)=0;
    ObjPoints.at<float>(1,0)=halfWidthSize;
    ObjPoints.at<float>(1,1)=-halfHeightSize;
    ObjPoints.at<float>(1,2)=0;
    ObjPoints.at<float>(2,0)=halfWidthSize;
    ObjPoints.at<float>(2,1)=halfHeightSize;
    ObjPoints.at<float>(2,2)=0;
    ObjPoints.at<float>(3,0)=-halfWidthSize;
    ObjPoints.at<float>(3,1)=halfHeightSize;
    ObjPoints.at<float>(3,2)=0;

    cv::Mat ImagePoints(4,2,CV_32FC1);

    //Set image points from the marker

    if( (distance_0_1 + distance_2_3) < (distance_1_2 + distance_3_0) ){
      for (int c=0;c<4;c++){
        ImagePoints.at<float>(c,0) = ((*this)[c%4].x);
        ImagePoints.at<float>(c,1) = ((*this)[c%4].y);
      }
    }
    else{
      ImagePoints.at<float>(0,0) = ((*this)[3].x);
      ImagePoints.at<float>(0,1) = ((*this)[3].y);
      ImagePoints.at<float>(1,0) = ((*this)[0].x);
      ImagePoints.at<float>(1,1) = ((*this)[0].y);
      ImagePoints.at<float>(2,0) = ((*this)[1].x);
      ImagePoints.at<float>(2,1) = ((*this)[1].y);
      ImagePoints.at<float>(3,0) = ((*this)[2].x);
      ImagePoints.at<float>(3,1) = ((*this)[2].y);
    }

    cv::Mat camMatrix = camera.getDistortedCamMatrix();
    cv::Mat distCoeff = camera.getDistCoeffs();
    cv::Mat raux,taux;
    cv::solvePnP(ObjPoints, ImagePoints, camMatrix, distCoeff,raux,taux);

    paperSize.width = paperSizeMeters.width;
    paperSize.height = paperSizeMeters.height;


    cv::Mat rotObjToProj, transObjToProj;

    cv::composeRT(raux, taux,
                  cameraProjector.getCamToProjRotation(), cameraProjector.getCamToProjTranslation(),
                  rotObjToProj, transObjToProj);

    //vector<cv::Point2f> out;

    //cv::projectPoints(cv::Mat(ObjPoints),
    //	rotObjToProj, transObjToProj,
    //		projector.getDistortedIntrinsics().getCameraMatrix(),
    //		projector.getDistCoeffs(),
    //		out);

    //calibrationProjector.setCandidateImagePoints(out);

    rotObjToProj.convertTo(rotVec,CV_32F);
    transObjToProj.convertTo(transVec ,CV_32F);

    //rotate the X axis so that Y is perpendicular to the marker plane
    if (setYPerperdicular)
      rotateXAxis(rotVec);

    //cout<<(*this)<<endl;
  }


  /**
   */
  void Paper::calculateScreenExtrinsics(cv::Size paperSizeMeters, CameraModel& camera, bool setYPerperdicular) throw(cv::Exception){
    if (!isValid())
      throw cv::Exception(9004,"!isValid(): invalid paper. It is not possible to calculate extrinsics","calculateExtrinsics",__FILE__,__LINE__);
    if (paperSizeMeters.width <= 0 || paperSizeMeters.height <= 0)
      throw cv::Exception(9004,"paperSize <=0: invalid paper size","calculateExtrinsics",__FILE__,__LINE__);
    if (!camera.isValid())
      throw cv::Exception(9004,"invalid camera parameters","calculateExtrinsics",__FILE__,__LINE__);

    float halfWidthSize  =  paperSizeMeters.width/2.;
    float halfHeightSize =  paperSizeMeters.height/2.;

    double  distance_0_1 = cv::norm( (*this)[0] - (*this)[1] );
    double  distance_1_2 = cv::norm( (*this)[1] - (*this)[2] );
    double  distance_2_3 = cv::norm( (*this)[2] - (*this)[3] );
    double  distance_3_0 = cv::norm( (*this)[3] - (*this)[0] );


    //cout << "distance_0_1: "<<  distance_0_1 << endl;
    //cout << "distance_1_2: "<<  distance_1_2 << endl;
    //cout << "distance_2_3: "<<  distance_2_3 << endl;
    //cout << "distance_3_0: "<<  distance_3_0 << endl;

    //cout << "distance_01-23: "<<  (distance_0_1 + distance_2_3) << endl;
    //cout << "distance_12-30: "<<  (distance_1_2 + distance_3_0) << endl;

    cv::Mat ObjPoints(4,3,CV_32FC1);

    ObjPoints.at<float>(0,0)=-halfWidthSize;
    ObjPoints.at<float>(0,1)=-halfHeightSize;
    ObjPoints.at<float>(0,2)=0;
    ObjPoints.at<float>(1,0)=halfWidthSize;
    ObjPoints.at<float>(1,1)=-halfHeightSize;
    ObjPoints.at<float>(1,2)=0;
    ObjPoints.at<float>(2,0)=halfWidthSize;
    ObjPoints.at<float>(2,1)=halfHeightSize;
    ObjPoints.at<float>(2,2)=0;
    ObjPoints.at<float>(3,0)=-halfWidthSize;
    ObjPoints.at<float>(3,1)=halfHeightSize;
    ObjPoints.at<float>(3,2)=0;

    cv::Mat ImagePoints(4,2,CV_32FC1);

    //Set image points from the marker

    if( (distance_0_1 + distance_2_3) < (distance_1_2 + distance_3_0) ){
      for (int c=0;c<4;c++){
        ImagePoints.at<float>(c,0) = ((*this)[c%4].x);
        ImagePoints.at<float>(c,1) = ((*this)[c%4].y);
      }
    }
    else{
      ImagePoints.at<float>(0,0) = ((*this)[3].x);
      ImagePoints.at<float>(0,1) = ((*this)[3].y);
      ImagePoints.at<float>(1,0) = ((*this)[0].x);
      ImagePoints.at<float>(1,1) = ((*this)[0].y);
      ImagePoints.at<float>(2,0) = ((*this)[1].x);
      ImagePoints.at<float>(2,1) = ((*this)[1].y);
      ImagePoints.at<float>(3,0) = ((*this)[2].x);
      ImagePoints.at<float>(3,1) = ((*this)[2].y);
    }

    cv::Mat camMatrix = camera.getDistortedCamMatrix();
    cv::Mat distCoeff = camera.getDistCoeffs();
    cv::Mat raux,taux;
    cv::solvePnP(ObjPoints, ImagePoints, camMatrix, distCoeff,raux,taux);
    raux.convertTo(rotVec,CV_32F);
    taux.convertTo(transVec,CV_32F);

    //rotate the X axis so that Y is perpendicular to the marker plane
    if (setYPerperdicular)
      rotateXAxis(rotVec);

    paperSize.width = paperSizeMeters.width;
    paperSize.height = paperSizeMeters.height;
    //cout<<(*this)<<endl;
  }

  /**
   */
  void Paper::rotateXAxis(Mat &rotation){
    cv::Mat rot3x3(3,3,CV_32F);
    Rodrigues(rotation, rot3x3);

    //create a rotation matrix for x axis
    cv::Mat rotX = cv::Mat::eye(3,3,CV_32F);
    float angleRad = M_PI/2;
    rotX.at<float>(1,1)=cos(angleRad);
    rotX.at<float>(1,2)=-sin(angleRad);
    rotX.at<float>(2,1)=sin(angleRad);
    rotX.at<float>(2,2)=cos(angleRad);

    //now multiply
    rot3x3 = rot3x3 * rotX;
    //finally, the the rodrigues back
    Rodrigues(rot3x3,rotation);
  }

  /**
   *
   */
  cv::Point2f Paper::getCenter()const{
    cv::Point2f cent(0,0);
    for(size_t i=0; i<size(); i++){
      cent.x+=(*this)[i].x;
      cent.y+=(*this)[i].y;
    }
    cent.x/= float(size());
    cent.y/= float(size());
    return cent;
  }

  /**
   */
  float Paper::getArea()const{
    assert(size() == 4);
    //use the cross products
    cv::Point2f v01=(*this)[1]-(*this)[0];
    cv::Point2f v03=(*this)[3]-(*this)[0];
    float area1 = fabs(v01.x*v03.y - v01.y*v03.x);
    cv::Point2f v21=(*this)[1]-(*this)[2];
    cv::Point2f v23=(*this)[3]-(*this)[2];
    float area2 = fabs(v21.x*v23.y - v21.y*v23.x);
    return (area2 + area1)/2.;
  }

  /**
   */
  float Paper::getPerimeter() const{
    assert(size() == 4);
    float sum=0;
    for(int i=0; i<4; i++)
      sum += norm((*this)[i]-(*this)[(i+1)%4]);
    return sum;
  }

}
