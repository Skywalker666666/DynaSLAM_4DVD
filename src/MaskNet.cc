/**
* This file is part of DynaSLAM.
*
* Copyright (C) 2018 Berta Bescos <bbescos at unizar dot es> (University of Zaragoza)
* For more information see <https://github.com/bertabescos/DynaSLAM>.
*
*/

#include "MaskNet.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <dirent.h>
#include <errno.h>

namespace DynaSLAM
{

#define U_SEGSt(a)\
    gettimeofday(&tvsv,0);\
    a = tvsv.tv_sec + tvsv.tv_usec/1000000.0
struct timeval tvsv;
double t1sv, t2sv,t0sv,t3sv;
void tic_initsv(){U_SEGSt(t0sv);}
void toc_finalsv(double &time){U_SEGSt(t3sv); time =  (t3sv- t0sv)/1;}
void ticsv(){U_SEGSt(t1sv);}
void tocsv(){U_SEGSt(t2sv);}
// std::cout << (t2sv - t1sv)/1 << std::endl;}

SegmentDynObject::SegmentDynObject(){
    std::cout << "Importing Mask R-CNN Settings..." << std::endl;
    ImportSettings();
    std::string x;
    setenv("PYTHONPATH", this->py_path.c_str(), 1);
    x = getenv("PYTHONPATH");
    Py_Initialize();
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append(\"/usr/bin/python2.7\")");
    import_array();

    std::cout << "py env: " << x << std::endl;
    std::cout << "Py_Initialized is over..." << std::endl;
    this->cvt = new NDArrayConverter();
    this->py_module = PyImport_ImportModule(this->module_name.c_str());
    assert(this->py_module != NULL);
    std::cout << "module is imported..." << std::endl;
    this->py_class = PyObject_GetAttrString(this->py_module, this->class_name.c_str());
    assert(this->py_class != NULL);
    std::cout << "py object is given..." << std::endl;
    this->net = PyInstance_New(this->py_class, NULL, NULL);
    assert(this->net != NULL);
    std::cout << "Creating net instance..." << std::endl;
    cv::Mat image  = cv::Mat::zeros(480,640,CV_8UC3); //Be careful with size!!
    std::cout << "Loading net parameters..." << std::endl;
    GetSegmentation(image);
}

SegmentDynObject::~SegmentDynObject(){
    delete this->py_module;
    delete this->py_class;
    delete this->net;
    delete this->cvt;
}

cv::Mat SegmentDynObject::GetSegmentation(cv::Mat &image,std::string dir, std::string name){
    cv::Mat seg = cv::imread(dir+"/"+name,CV_LOAD_IMAGE_UNCHANGED);
    std::cout << "GetSegmentation routine" << std::endl;
    std::cout << dir << std::endl;
    std::cout << dir+"/"+name << std::endl;
    std::cout << seg.empty() << std::endl;

    if(seg.empty()){
        std::cout << "seg empty() call 1" << std::endl;
        PyObject* py_image = cvt->toNDArray(image.clone());

        if (py_image     == NULL) {
            std::cout << "py_image  is empty" << std::endl;
            exit(-1);
        }
        else
        {
            std::cout << "py_image is successful" << std::endl;
        }
       
        //cv::imshow("MaskNet.cc Display window X", image);
        //int k = cv::waitKey(0);

        //PyObject* py_image_empty;
        //std::cout << "seg empty() call 1_1" << std::endl;
        //seg = cvt->toMat(py_image).clone();
        //seg = cvt->toMat(py_image_empty).clone();
        //std::cout << "seg empty() call 1_2" << std::endl;

        //seg.cv::Mat::convertTo(seg,CV_8U);//0 background y 1 foreground
        //std::cout << "seg empty() call 1_3" << std::endl;
        //cv::imwrite(dir+"/"+name,seg);

        std::cout << "seg empty() call 2" << std::endl;
        assert(py_image != NULL);
        std::cout << "seg empty() call 2_2" << std::endl;
        PyObject* py_mask_image = PyObject_CallMethod(this->net, (char *)this->get_dyn_seg.c_str(),"(O)", py_image);
        std::cout << "seg empty() call 3" << std::endl;
        seg = cvt->toMat(py_mask_image).clone();
        seg.cv::Mat::convertTo(seg,CV_8U);//0 background y 1 foreground
        std::cout << "seg empty() call 4" << std::endl;
        if(dir.compare("no_save")!=0){
            DIR* _dir = opendir(dir.c_str());
            if (_dir) {closedir(_dir);}
            else if (ENOENT == errno)
            {
                const int check = mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                if (check == -1) {
                    std::string str = dir;
                    str.replace(str.end() - 6, str.end(), "");
                    mkdir(str.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                }
            }
            cv::imwrite(dir+"/"+name,seg);
        }
    }
    return seg;
}

void SegmentDynObject::ImportSettings(){
    std::string strSettingsFile = "./Examples/RGB-D/MaskSettings.yaml";
    cv::FileStorage fs(strSettingsFile.c_str(), cv::FileStorage::READ);
    fs["py_path"] >> this->py_path;
    fs["module_name"] >> this->module_name;
    fs["class_name"] >> this->class_name;
    fs["get_dyn_seg"] >> this->get_dyn_seg;

    std::cout << "    py_path: "<< this->py_path << std::endl;
    std::cout << "    module_name: "<< this->module_name << std::endl;
    std::cout << "    class_name: "<< this->class_name << std::endl;
    std::cout << "    get_dyn_seg: "<< this->get_dyn_seg << std::endl;
}


}






















