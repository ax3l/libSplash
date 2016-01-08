/**
 * Copyright 2013 Felix Schmitt
 *
 * This file is part of libSplash. 
 * 
 * libSplash is free software: you can redistribute it and/or modify 
 * it under the terms of of either the GNU General Public License or 
 * the GNU Lesser General Public License as published by 
 * the Free Software Foundation, either version 3 of the License, or 
 * (at your option) any later version. 
 * libSplash is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License and the GNU Lesser General Public License 
 * for more details. 
 * 
 * You should have received a copy of the GNU General Public License 
 * and the GNU Lesser General Public License along with libSplash. 
 * If not, see <http://www.gnu.org/licenses/>. 
 */

#ifndef DCPARALLELDATASET_HPP
#define	DCPARALLELDATASET_HPP

#include "splash/core/DCDataSet.hpp"

#include <iostream>

namespace splash
{

    /**
     * \cond HIDDEN_SYMBOLS
     */
    class DCParallelDataSet : public DCDataSet
    {
    public:

        DCParallelDataSet(const std::string name) :
        DCDataSet(name)
        {
            dsetWriteProperties = H5Pcreate(H5P_DATASET_XFER);
            H5Pset_dxpl_mpio(dsetWriteProperties, H5FD_MPIO_COLLECTIVE);

            dsetReadProperties = H5Pcreate(H5P_DATASET_XFER);
            H5Pset_dxpl_mpio(dsetReadProperties, H5FD_MPIO_COLLECTIVE);

            checkExistence = false;
        }

        virtual ~DCParallelDataSet()
        {
            H5D_mpio_actual_chunk_opt_mode_t actual_chunk_opt_mode_write;
            H5Pget_mpio_actual_chunk_opt_mode( dsetWriteProperties,
                                               &actual_chunk_opt_mode_write );
            switch( actual_chunk_opt_mode_write ) {
                case H5D_MPIO_NO_CHUNK_OPTIMIZATION : std::cerr << "[CHUNK-OPT] NO OPTIMIZATION" << std::endl;
                                                      break;
                case H5D_MPIO_LINK_CHUNK : std::cerr << "[CHUNK-OPT] LINK" << std::endl;
                                           break;
                case H5D_MPIO_MULTI_CHUNK : std::cerr << "[CHUNK-OPT] MULTI" << std::endl;
                                            break;
                default : std::cerr << "[CHUNK-OPT] " << actual_chunk_opt_mode_write << std::endl;
                          break;
            }

            H5D_mpio_actual_io_mode_t actual_io_mode_write;
            H5Pget_mpio_actual_io_mode( dsetWriteProperties,
                                        &actual_io_mode_write );

            switch( actual_io_mode_write ) {
                case H5D_MPIO_NO_COLLECTIVE : std::cerr << "[MODE] NO COLLECTIVE" << std::endl;
                                              break;
                case H5D_MPIO_CHUNK_INDEPENDENT : std::cerr << "[MODE] CHUNK INDEPENDENT" << std::endl;
                                              break;
                case H5D_MPIO_CHUNK_COLLECTIVE : std::cerr << "[MODE] CHUNK COLLECTIVE" << std::endl;
                                           break;
                case H5D_MPIO_CHUNK_MIXED : std::cerr << "[MODE] CHUNK MIXED" << std::endl;
                                           break;
                case H5D_MPIO_CONTIGUOUS_COLLECTIVE : std::cerr << "[MODE] CONTIGUOUS COLLECTIVE" << std::endl;
                                           break;
                default: std::cerr << "[MODE] " << actual_io_mode_write << std::endl;
                         break;
            }

            H5D_mpio_actual_io_mode_t actual_io_mode_read;
            H5Pget_mpio_actual_io_mode( dsetReadProperties,
                                        &actual_io_mode_read);
            // H5Pget_mpio_no_collective_cause( hid_t dxpl_id, uint32_t*
            //   local_no_collective_cause, uint32_t* global_no_collective_cause);

            H5Pclose(dsetWriteProperties);
            H5Pclose(dsetReadProperties);
        }

        void setWriteIndependent()
        {
            H5Pset_dxpl_mpio(dsetWriteProperties, H5FD_MPIO_INDEPENDENT);
        }
    };
    /**
     * \endcond
     */
}

#endif	/* DCPARALLELDATASET_HPP */

