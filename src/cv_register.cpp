//#############################################################################
//
//	cv_register.cpp
//
//#############################################################################
#include <cstdlib>
#include <iostream>
#include "cv_parser.h"
#include "cv_prepare.h"
#include "cv_register.h"

namespace mycv
{

//#############################################################################
//
//  applyICP(): apply ICP algorithm on 2 point clouds
//
//#############################################################################
int applyICP(	const cv::Mat 	&dst_p_mat,
				const cv::Mat 	&src_p_mat,
				double 			&score,
				cv::Mat 		&out_p_mat,
				cv::Matx44d 	&transformation)
{
	//--- Compute normals
	cv::Mat 		dst_pn_mat;
	cv::Mat 		src_pn_mat;
	const int 		NbrNum = 20;
	const cv::Vec3f	origin(0.0, 0.0, 0.0);	
	if (cv::ppf_match_3d::computeNormalsPC3d(	dst_p_mat,
												dst_pn_mat,
												NbrNum,
												true,
												origin) != 1)
	{
		std::cout << "ERROR: cv::ppf_match_3d::computeNormalsPC3d() on dst_p_mat" << std::endl;
		return EXIT_FAILURE;
	}
	if (cv::ppf_match_3d::computeNormalsPC3d(	src_p_mat,
												src_pn_mat,
												NbrNum,
												true,
												origin) != 1)
	{
		std::cout << "ERROR: cv::ppf_match_3d::computeNormalsPC3d() on src_p_mat" << std::endl;
		return EXIT_FAILURE;		
	}

	//--- Sample the point cloud
	float 		grid_step 	= 0.01;
	cv::Mat 	dst_spn_mat;
	cv::Mat 	src_spn_mat;

	mycv::sampleCloudGridstep(dst_pn_mat, grid_step, dst_spn_mat);
	mycv::sampleCloudGridstep(src_pn_mat, grid_step, src_spn_mat);

	//--- Apply Iterative Closet Point algorithm
	int 	maxIterations	= 100;
	float 	tolerance		= 0.005f;
	float 	rejectionScale	= 1.0f;
	int 	numLevels		= 4;

	cv::ppf_match_3d::ICP 	icp(maxIterations,
								tolerance,
								rejectionScale,
								numLevels);

	if (icp.registerModelToScene(	src_spn_mat,
									dst_spn_mat,
									score,
									transformation) != 0)
	{
		std::cout << "ERROR: cv::ppf_match_3d::ICP::registerModelToScene()" << std::endl;
		return EXIT_FAILURE;		
	}
	out_p_mat = cv::ppf_match_3d::transformPCPose(src_p_mat, transformation);

	return EXIT_SUCCESS;
}

//#############################################################################
//
//  applyICP(): apply ICP algorithm on 2 point clouds
//
//#############################################################################
int applyICP(	const std::vector<cv::Point3d> 	&dst_p_vec,
				const std::vector<cv::Point3d> 	&src_p_vec,
				double 							&score,
				cv::Mat 						&out_p_mat,
				cv::Matx44d 					&transformation)
{
	cv::Mat dst_p_mat;
	cv::Mat src_p_mat;

	convertVECtoMAT(dst_p_vec, dst_p_mat);
	convertVECtoMAT(src_p_vec, src_p_mat);

	if (applyICP(	dst_p_mat,
					src_p_mat,
					score,
					out_p_mat,
					transformation) == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

//#############################################################################
//
//  applyPPF(): apply PPF on 2 point clouds
//
//#############################################################################
int applyPPF(	const cv::Mat 			&dst_p_mat,
				const cv::Mat 			&src_p_mat,
				const size_t 			&head,
				std::vector<pose_ptr> 	&pose_ptrs)
{	
	//--- Compute normals
	cv::Mat 		dst_pn_mat;
	cv::Mat 		src_pn_mat;
	const int 		NbrNum = 10;
	const cv::Vec3f	origin(0.0, 0.0, 0.0);	
	if (cv::ppf_match_3d::computeNormalsPC3d(	dst_p_mat,
												dst_pn_mat,
												NbrNum,
												true,
												origin) != 1)
	{
		std::cout << "ERROR: cv::ppf_match_3d::computeNormalsPC3d() on dst_p_mat" << std::endl;
		return EXIT_FAILURE;
	}
	if (cv::ppf_match_3d::computeNormalsPC3d(	src_p_mat,
												src_pn_mat,
												NbrNum,
												true,
												origin) != 1)
	{
		std::cout << "ERROR: cv::ppf_match_3d::computeNormalsPC3d() on src_p_mat" << std::endl;
		return EXIT_FAILURE;		
	}

	//--- Train source cloud and match target cloud
	cv::ppf_match_3d::PPF3DDetector detector(0.05, 0.05);
	std::vector<pose_ptr>			full_pose_ptrs;
	size_t 							pose_ptrs_size;
	detector.trainModel(src_pn_mat);
	std::cout << "complete trainModel()" << std::endl;
	detector.match(dst_pn_mat, full_pose_ptrs, 1.0/40.0, 0.05);
	std::cout << "complete match()" << std::endl;

	//--- Get pose
	pose_ptrs.clear();
	pose_ptrs_size = std::min<size_t>(full_pose_ptrs.size(),head);
	pose_ptrs.assign(full_pose_ptrs.begin(),full_pose_ptrs.begin()+pose_ptrs_size);

	return EXIT_SUCCESS;
}

//#############################################################################
//
//  applyPPFICP(): apply PPF+ICP algorithm on 2 point clouds
//
//#############################################################################
int applyPPFICP(const cv::Mat 			&dst_p_mat,
				const cv::Mat 			&src_p_mat,
				const int 				&head,
				std::vector<pose_ptr> 	&pose_ptrs,
				std::vector<cv::Mat>	&out_p_mats)
{
	cv::ppf_match_3d::ICP 	icp(100, 0.005f, 1.0f, 4);
	size_t 					out_count;
	cv::Mat 				out_p_mat;

	pose_ptrs.clear();
	applyPPF(	dst_p_mat,
				src_p_mat,
				head,
				pose_ptrs);

	std::cout << "complete applyPPF()" << std::endl;

	icp.registerModelToScene(	src_p_mat,
								dst_p_mat,
								pose_ptrs);

	std::cout << "complete registerModelToScene()" << std::endl;

	out_count = pose_ptrs.size();
	out_p_mats.clear();
	out_p_mats.reserve(out_count);
	for (size_t i = 0; i < out_count; ++i)
	{
		out_p_mat = cv::ppf_match_3d::transformPCPose(src_p_mat, pose_ptrs.at(i)->pose);
		out_p_mats.push_back(out_p_mat);
	}

	return EXIT_SUCCESS;
}

//#############################################################################
//
//  applyPPFICP(): apply PPF+ICP algorithm on 2 point clouds
//
//#############################################################################
int applyPPFICP(const cv::Mat 			&dst_p_mat,
				const cv::Mat 			&src_p_mat,
				pose_ptr 				&first_pose_ptr,
				cv::Mat					&out_p_mat)
{
	std::vector<pose_ptr> 	pose_ptrs;
	cv::ppf_match_3d::ICP 	icp(100, 0.005f, 1.0f, 4);

	pose_ptrs.clear();
	applyPPF(	dst_p_mat,
				src_p_mat,
				1,
				pose_ptrs);

	std::cout << "complete applyPPF()" << std::endl;

	icp.registerModelToScene(	src_p_mat,
								dst_p_mat,
								pose_ptrs);

	std::cout << "complete registerModelToScene()" << std::endl;

	first_pose_ptr = pose_ptrs.at(0);
	out_p_mat = cv::ppf_match_3d::transformPCPose(src_p_mat, first_pose_ptr->pose);

	return EXIT_SUCCESS;
}

} //--- namespace mycv