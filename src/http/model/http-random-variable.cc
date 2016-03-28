/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Yufei Cheng
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Yufei Cheng   <yfcheng@ittc.ku.edu>
 *
 * James P.G. Sterbenz <jpgs@ittc.ku.edu>, director
 * ResiliNets Research Group  http://wiki.ittc.ku.edu/resilinets
 * Information and Telecommunication Technology Center (ITTC)
 * and Department of Electrical Engineering and Computer Science
 * The University of Kansas Lawrence, KS USA.
 *
 * Work supported in part by NSF FIND (Future Internet Design) Program
 * under grant CNS-0626918 (Postmodern Internet Architecture),
 * NSF grant CNS-1050226 (Multilayer Network Resilience Analysis and Experimentation on GENI),
 * US Department of Defense (DoD), and ITTC at The University of Kansas.
 *
 * Notification:
 * Statistics Research, Bell Labs, Lucent Technologies and The University of North Carolina at Chapel Hill
 */

#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sstream>
#include <vector>

#include "ns3/log.h"
#include "ns3/test.h"
#include "ns3/assert.h"
#include "ns3/config.h"
#include "ns3/integer.h"
#include "ns3/fatal-error.h"

#include "http-random-variable.h"

using namespace std;

namespace ns3 {
namespace http {
//-------------------------------------------------------------
//-------------------------------------------------------------
//HttpRandomVariableBase
HttpRandomVariableBase::HttpRandomVariableBase ()
{
}

HttpRandomVariableBase::~HttpRandomVariableBase ()
{
}

double HttpRandomVariableBase::Logit (double m_x)
{
  return log (m_x / (1 - m_x)) / LOG2;
}

double HttpRandomVariableBase::LogitInv (double m_x)
{
  return pow (2, m_x) / (1 + pow (2.0, m_x));
}

double HttpRandomVariableBase::qWeibull (double p, double shape, double scale)
{
  return (pow (-log (1 - p), 1 / shape) * scale);
}

double HttpRandomVariableBase::pNorm (double q)
{
  if (q == 0)
    {
      return(0.5);
    }
  else if (q > 0)
    {
      return((1 + erf (q / ROOT_2)) / 2);
    }
  else
    {
      return(erfc (-q / ROOT_2) / 2);
    }
}

uint32_t HttpRandomVariableBase::rBernoulli (double p)
{
  double x;
  uint32_t z;

  x = m_uniform.GetValue ();
  if (x <= p)
    {
      z = 1;
    }
  else
    {
      z = 0;
    }

  return z;
}

double HttpRandomVariableBase::Gammln (double xx)
{
  uint32_t j;
  double x, y, tmp, ser;
  static double cof[6] = {
    76.18009172947146,-86.50532032941677,
    24.01409824083091,-1.231739572450155,
    0.1208650973866179e-2,-0.5395239384953e-5
  };
  y = x = xx;
  tmp = x + 5.5;
  tmp -= (x + 0.5) * log (tmp);
  ser = 1.000000000190015;
  for (j = 0; j <= 5; j++)
    {
      ser += cof[j] / ++y;
    }

  return (-tmp + log (2.5066282746310005 * ser / x));
}
//-------------------------------------------------------------
//-------------------------------------------------------------
//Slope
Slope::Slope (const double *x, const double *y, uint32_t n)
  : m_x ((double*)x),
    m_y ((double*)y),
    m_nSteps (n)
{
  NS_ASSERT (n >= 2);
  m_slope = new double[n];
  for (uint32_t i = 0; i < n - 1; i++)
    {
      m_slope[i] = (y[i + 1] - y[i]) / (x[i + 1] - x[i]);
    }
  m_slope[n - 1] = m_slope[n - 2];
}

Slope::~Slope ()
{
  delete[] m_slope;
}

double Slope::LinearInterpolate (double xnew)
{
  uint32_t i;

  // we should use binary search if nsteps is large, but for now...
  for (i = 0; i < m_nSteps - 2; i++)
    {
      if (xnew < m_x[i + 1])
        {
          return (m_y[i] + m_slope[i] * (xnew - m_x[i]));
        }
    }

  return (m_y[m_nSteps - 1] + m_slope[m_nSteps - 1] * (xnew - m_x[m_nSteps - 1]));
}
//-------------------------------------------------------------
//-------------------------------------------------------------
//fArimaImpl
fArimaImpl::fArimaImpl (double d, uint32_t N, uint32_t pAR, uint32_t qMA)
  : m_N (N),
    m_pAR (pAR),
    m_qMA (qMA),
    m_d (d)
{
  NS_ASSERT (N && qMA);

  if (pAR)
    {
      m_AR = new double[pAR];
      m_y = new double [pAR];
    }

  if (qMA)
    {
      m_MA = new double[qMA];
    }
  m_x = new double [N];
  m_phi = new double [N];

  /**
   * Use 1-theta[1] B-theta[2] B^2 - ..., and
   * our model is epsilon_j + epsilon_(j-1)
   */
  m_MA[0] = -1;

  for (uint32_t i = 0; i < pAR; i++)
    {
      m_y[i] = 0.0;
    }

  m_t = m_tmod = 0;

  // Skip the first few in simulation to stabilize
  while (m_t < N + pAR + qMA)
    {
      NextLow ();
    }
}

fArimaImpl::~fArimaImpl ()
{
  if (m_pAR > 0)
    {
      delete[] m_AR;
      delete[] m_y;
    }
  if (m_qMA > 0)
    {
      delete[] m_MA;
    }
  delete[] m_phi;
  delete[] m_x;
}

double fArimaImpl::NextLow ()
{
  uint32_t j;
  double mt, xt, yt, a, b, v0;

  if (m_t == 0)
    {
      // d_0.25; gamma(1-2*d)/(gamma(1-d))^2 => 1.180341
      v0 = Gammln (1.0 - 2.0 * m_d) - 2.0 * Gammln (1.0 - m_d);
      v0 = exp (v0);
      // phi[0] is not used, so we use it for vt
      m_phi[0] = v0;
      m_x[m_t] = m_normal.GetValue () * pow (v0, 0.5);
    }
  else if (m_t < m_N)
    {
      // get phi_tt
      m_phi[m_t] = m_d / (m_t - m_d);
      // get phi_tj
      for (j = 1; j < m_t; j++)
        {
          if (j <= m_t - j)
            {
              a = m_phi[j] - m_phi[m_t] * m_phi[m_t - j];
              b = m_phi[m_t - j] - m_phi[m_t] * m_phi[j];
              m_phi[j] = a;
              m_phi[m_t - j] = b;
            }
        }
      // get vt (phi[0])
      // v[t] = (1-phi_tt^2) * v[t-1]
      m_phi[0] = (1 - m_phi[m_t] * m_phi[m_t]) * m_phi[0];
    }

  // get m_t, v_t
  mt = 0;
  for (j = 1; j <= Min (m_t,m_N - 1); j++)
    {
      mt += m_phi[j] * m_x[(m_t - j) % m_N];
    }

  // get xt
  xt = m_normal.GetValue () * pow (m_phi[0], 0.5) + mt;
  m_x[m_t % m_N] = xt;

  /**
   * add AR, MA parts only after enough points in the error
   * fARIMA process
   */
  yt = xt;
  if (m_t > m_qMA)
    {
      if (m_qMA > 0)
        {
          for (j = 0; j < m_qMA; j++)
            {
              yt -= m_x[(m_t - 1 - j) % m_N] * m_MA[j];
            }
        }
    }

  m_t++;
  m_tmod = (++m_tmod == m_N) ? 0 : m_tmod;

  return (yt);
}

double fArimaImpl::GetValue ()
{
  double mt, xt, yt;
  uint32_t j, m;

  // get m_t, v_t
  mt = 0;
  for (j = 1; j <= m_tmod; j++)
    {
      mt += m_phi[j] * m_x[m_tmod - j];
    }

  for (j = m_tmod + 1; j <= m_N - 1; j++)
    {
      mt += m_phi[j] * m_x[m_N + m_tmod - j];
    }

  // get xt
  xt = m_normal.GetValue () * pow (m_phi[0], 0.5) + mt;
  m_x[m_tmod] = xt;

  /**
   * add AR, MA parts only after enough points in the error
   * fARIMA process
   */
  yt = xt;
  if (m_t > m_qMA)
    {
      if (m_qMA > 0)
        {
          m = (m_tmod < m_qMA) ? m_tmod : m_qMA;
          for (j = 0; j < m; j++)
            {
              yt -= m_x[m_tmod - 1 - j] * m_MA[j];
            }
          for (j = m; j < m_qMA; j++)
            {
              yt -= m_x[m_N + m_tmod - 1 - j] * m_MA[j];
            }
        }
    }

  m_t++;
  m_tmod = (++m_tmod == m_N) ? 0 : m_tmod;

  return (yt);
}

double fArimaImpl::Next ()
{
  double mt, xt, yt;
  uint32_t j, m;

  // get m_t, v_t
  mt = 0;
  for (j = 1; j <= m_tmod; j++)
    {
      mt += m_phi[j] * m_x[m_tmod - j];
    }

  for (j = m_tmod + 1; j <= m_N - 1; j++)
    {
      mt += m_phi[j] * m_x[m_N + m_tmod - j];
    }

  // get xt
  xt = m_normal.GetValue () * pow (m_phi[0], 0.5) + mt;
  m_x[m_tmod] = xt;

  /**
   * add AR, MA parts only after enough points in the error
   * fARIMA process
   */
  yt = xt;
  if (m_t > m_qMA)
    {
      if (m_qMA > 0)
        {
          m = (m_tmod < m_qMA) ? m_tmod : m_qMA;
          for (j = 0; j < m; j++)
            {
              yt -= m_x[m_tmod - 1 - j] * m_MA[j];
            }
          for (j = m; j < m_qMA; j++)
            {
              yt -= m_x[m_N + m_tmod - 1 - j] * m_MA[j];
            }
        }
    }

  m_t++;
  m_tmod = (++m_tmod == m_N) ? 0 : m_tmod;

  return (yt);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//HttpFileSizeRandomVariableImpl

HttpFileSizeRandomVariableImpl::HttpFileSizeRandomVariableImpl (double rate, uint32_t type)
  : m_rate (rate),
    m_type (type),
    m_const (1),
    m_mean (0),
    m_fArima (NULL)
{
  Initialize ();
}

HttpFileSizeRandomVariableImpl::~HttpFileSizeRandomVariableImpl ()
{
  if (m_fArima)
    {
      delete m_fArima;
    }
}

void HttpFileSizeRandomVariableImpl::Initialize ()
{
  double sigmaTotal;
  struct ArimaParams *arima = &fileSizeArimaParams;
  double d = arima->d;
  uint32_t N = arima->N;

  /// fsize0 run length
  m_state = 0;
  m_runlen = 0;

  /// fsize1
  sigmaTotal = 1.0;
  /*
   * Modeled by an FSD model.  The estimated value of d is 0.31 and the logistic model is
   * LogitInv (0.123 + 0.494 * log (m_rate) / LOG2)
   * This is a discrete Weibull distribution, a probability distribution on the positive
   * integers.
   */
  m_varRatio = LogitInv (REQUEST_VARRATIO_INTERCEPT +
                         REQUEST_VARRATIO_SLOPE * log (m_rate) / LOG2);
  m_sigmaNoise = sigmaTotal * pow (m_varRatio,0.5);
  m_sigmaEpsilon = sigmaTotal *
    pow (exp (2.0 * Gammln (1.0 - d) - Gammln (1.0 - 2.0 * d)) /
         (2 + 2 * d / (1 - d)) * (1 - m_varRatio), 0.5);

  /// fsize0 run length
  /// This m_shape is for not modified message
  /// The state 0 is for the not modified message
  m_shape[0] = LogitInv (0.718 + 0.357 * log (m_rate) / LOG2);
  /// This part is for the content objects
  /// The state 1 is for the content objects
  m_shape[1] = LogitInv (1.293 + 0.316 * log (m_rate) / LOG2);
  /// As Bernoulli series p goes to infinity, the scale parameters of the run lengths tend to
  /// -1/log(q) = 0.775, take 0.775 for not modified message
  m_scale[0] = 0.775;
  /// -1/log(1-q) = 3.11 as p goes to infinity, take 3.11 for content
  m_scale[1] = 3.11;

  /// simulate the arfima process
  m_fArima = new fArimaImpl (d, N);
}

uint32_t HttpFileSizeRandomVariableImpl::ResponseSize (uint32_t state)
{
  double yt, p;

  p = m_uniform.GetValue ();
  if (state == 1)
    {
      p = log (1 - p) / LOG2;
    }
  yt = fsize_invcdf[0][state].LinearInterpolate (p);
  if (state == 1)
    {
      yt = exp (LOG2 * yt);
    }

  return((uint32_t) yt);
}

uint32_t HttpFileSizeRandomVariableImpl::RequestSize (double p)
{
  uint32_t state;
  double yt;

  /// first convert this to conditional probability
  if (p > REQUEST_PROB_A)
    {
      state = 1;
      p = (p - REQUEST_PROB_A) / (1 - REQUEST_PROB_A);
    }
  else
    {
      state = 0;
      p = p / REQUEST_PROB_A;
    }

  if (state == 1)
    {
      p = log (1 - p) / LOG2;
    }
  yt = fsize_invcdf[1][state].LinearInterpolate (p);
  if (state == 1)
    {
      yt = exp (LOG2 * yt);
    }

  return((uint32_t) yt);
}

double HttpFileSizeRandomVariableImpl::Average (void)
{
  /*
   * Give the average value of both request and response size, with the average response size larger than
   * that of request
   */
  if (m_type == HTTP_RESPONSE_SIZE)
    {
      return 2272;
    }
  else
    {
      return 423;
    }
}

double HttpFileSizeRandomVariableImpl::GetValue ()
{
  uint32_t yt;

  if (m_fArima == NULL)
    {
      Initialize ();
    }

  WeibullVariable weibull (1.0, m_shape[m_state], m_scale[m_state]);

  if (m_type == HTTP_RESPONSE_SIZE)
    {
      /// Regenerate the runlen if one run is finished, and the state will be 1 when runlen finished
      if (m_runlen <= 0)
        {
          m_state = 1 - m_state;
          m_runlen = (uint32_t) ceil (weibull.GetValue ());
        }
      yt = ResponseSize (m_state);
      m_runlen--;

      // adjust by m_const and m_mean (by default, this does nothing since m_const is set as 1)
      if (m_const == 0)
        {
          m_const = m_mean / Average ();
        }
      return (double) (yt * m_const);
    }
  else if (m_type == HTTP_REQUEST_SIZE)
    {
      double yx;
      yx = m_fArima->Next ();
      yx = yx * m_sigmaEpsilon + m_normal.GetValue () * m_sigmaNoise;
      yt = RequestSize (pNorm (yx));

      // adjust by m_const and m_mean (by default, this does nothing since m_const is set as 1)
      if (m_const == 0)
        {
          m_const = m_mean / Average ();
        }
      return (double) (yt * m_const);
    }
  return 0;
}

const double HttpFileSizeRandomVariableImpl::RESPONSE_INCDF_NOT_MODIFIED_X[] = {
  1e-04, 0.0102, 0.0203, 0.0304, 0.0405, 0.0506, 0.0607, 0.0708,
  0.0809, 0.091, 0.1011, 0.1112, 0.1213, 0.1314, 0.1415, 0.1516,
  0.1617, 0.1718, 0.1819, 0.192, 0.2021, 0.2122, 0.2223, 0.2324,
  0.2425, 0.2526, 0.2627, 0.2728, 0.2829, 0.293, 0.3031, 0.3132,
  0.3233, 0.3334, 0.3435, 0.3536, 0.3637, 0.3738, 0.3839, 0.394,
  0.4041, 0.4142, 0.4243, 0.4344, 0.4445, 0.4546, 0.4647, 0.4748,
  0.4849, 0.495, 0.5051, 0.5152, 0.5253, 0.5354, 0.5455, 0.5556,
  0.5657, 0.5758, 0.5859, 0.596, 0.6061, 0.6162, 0.6263, 0.6364,
  0.64649, 0.65659, 0.66669, 0.67679, 0.68689, 0.69699, 0.70709,
  0.71719, 0.72729, 0.73739, 0.74749, 0.75759, 0.76769, 0.77779,
  0.78789, 0.79799, 0.80809, 0.81819, 0.82829, 0.83839, 0.84849,
  0.85859, 0.86869, 0.87879, 0.88889, 0.89899, 0.90909, 0.91919,
  0.92929, 0.93939, 0.94949, 0.95959, 0.96969, 0.97979, 0.98989,
  0.99999
};

const double HttpFileSizeRandomVariableImpl::RESPONSE_INCDF_NOT_MODIFIED_Y[] = {
  4.645, 22.7357, 40.8041, 53.0108, 61.468, 64.4307, 67.3933, 69.6925,
  71.3981, 73.1037, 74.8092, 80.6677, 87.4875, 93.5693, 97.6514,
  101.249, 104.58, 107.021, 109.195, 111.087, 112.752, 114.273,
  115.545, 116.78, 118.014, 119.248, 120.483, 121.551, 122.51, 123.441,
  124.312, 125.119, 125.927, 126.734, 127.524, 128.288, 129.053,
  129.818, 130.582, 131.347, 132.143, 132.973, 133.835, 134.71,
  135.603, 136.537, 137.47, 138.404, 139.337, 140.28, 141.233, 142.186,
  143.139, 144.092, 145.045, 145.979, 146.904, 147.816, 148.71,
  149.589, 150.485, 151.478, 152.703, 153.928, 155.403, 157.294,
  159.389, 161.953, 165.018, 168.54, 172.7, 177.629, 183.365, 189.766,
  195.845, 200.636, 203.987, 206.43, 208.611, 211.114, 214.424,
  218.008, 221.925, 225.722, 229.242, 232.747, 236.235, 239.952,
  244.076, 248.68, 253.434, 257.857, 261.539, 264.471, 266.742,
  268.487, 269.793, 270.794, 271.571, 272.234
};

const double HttpFileSizeRandomVariableImpl::RESPONSE_INCDF_CONTENT_X[] = {
  -18.1793, -17.9957, -17.8121, -17.6285, -17.4449, -17.2613, -17.0777,
  -16.8941, -16.7104, -16.5268, -16.3432, -16.1596, -15.976, -15.7924,
  -15.6088, -15.4252, -15.2415, -15.0579, -14.8743, -14.6907, -14.5071,
  -14.3235, -14.1399, -13.9563, -13.7726, -13.589, -13.4054, -13.2218,
  -13.0382, -12.8546, -12.671, -12.4874, -12.3038, -12.1201, -11.9365,
  -11.7529, -11.5693, -11.3857, -11.2021, -11.0185, -10.8349, -10.6513,
  -10.4677, -10.284, -10.1004, -9.91682, -9.7332, -9.54959, -9.36598,
  -9.18237, -8.99876, -8.81515, -8.63154, -8.44792, -8.26431, -8.0807,
  -7.89709, -7.71348, -7.52987, -7.34626, -7.16265, -6.97903, -6.79542,
  -6.61181, -6.4282, -6.24459, -6.06098, -5.87737, -5.69376, -5.51014,
  -5.32653, -5.14292, -4.95931, -4.7757, -4.59209, -4.40848, -4.22486,
  -4.04125, -3.85764, -3.67403, -3.49042, -3.30681, -3.1232, -2.93959,
  -2.75597, -2.57236, -2.38875, -2.20514, -2.02153, -1.83792, -1.65431,
  -1.47069, -1.28708, -1.10347, -0.91986, -0.73625, -0.55264, -0.36903,
  -0.18542, -0.0018
};

const double HttpFileSizeRandomVariableImpl::RESPONSE_INCDF_CONTENT_Y[] = {
  25.8765, 25.7274, 25.5782, 25.4291, 25.2799, 25.1308, 24.9817,
  24.8325, 24.6834, 24.5342, 24.3851, 24.2359, 24.0868, 23.9377,
  23.7885, 23.6394, 23.4902, 23.3411, 23.1919, 23.0428, 22.8937,
  22.7445, 22.5954, 22.4462, 22.2971, 22.1479, 21.9988, 21.8497,
  21.7005, 21.5514, 21.4022, 21.2531, 21.104, 20.9548, 20.8057,
  20.6565, 20.5074, 20.3582, 20.2091, 20.06, 19.9108, 19.7617, 19.6125,
  19.4634, 19.3143, 19.1651, 19.016, 18.8668, 18.7177, 18.5685,
  18.4194, 18.2703, 18.1211, 17.972, 17.8228, 17.6737, 17.5245,
  17.3754, 17.2263, 17.0771, 16.928, 16.7788, 16.6297, 16.4806,
  16.3314, 16.1823, 16.0331, 15.8842, 15.739, 15.6026, 15.4804,
  15.3778, 15.3003, 15.2465, 15.2045, 15.1597, 15.0978, 15.0044,
  14.8675, 14.6932, 14.4968, 14.2932, 14.0976, 13.9248, 13.7783,
  13.6435, 13.5042, 13.3444, 13.1478, 12.9005, 12.6008, 12.2512,
  11.8545, 11.4133, 10.9303, 10.4104, 9.86075, 9.28886, 8.70213,
  8.10797
};

const double HttpFileSizeRandomVariableImpl::REQUEST_INCDF_NOT_MODIFIED_X[] = {
  3e-05, 0.01013, 0.02023, 0.03033, 0.04043, 0.05053, 0.06063, 0.07074,
  0.08084, 0.09094, 0.10104, 0.11114, 0.12124, 0.13134, 0.14144,
  0.15154, 0.16164, 0.17174, 0.18184, 0.19194, 0.20204, 0.21214,
  0.22225, 0.23235, 0.24245, 0.25255, 0.26265, 0.27275, 0.28285,
  0.29295, 0.30305, 0.31315, 0.32325, 0.33335, 0.34345, 0.35355,
  0.36365, 0.37376, 0.38386, 0.39396, 0.40406, 0.41416, 0.42426,
  0.43436, 0.44446, 0.45456, 0.46466, 0.47476, 0.48486, 0.49496,
  0.50506, 0.51516, 0.52526, 0.53537, 0.54547, 0.55557, 0.56567,
  0.57577, 0.58587, 0.59597, 0.60607, 0.61617, 0.62627, 0.63637,
  0.64647, 0.65657, 0.66667, 0.67677, 0.68688, 0.69698, 0.70708,
  0.71718, 0.72728, 0.73738, 0.74748, 0.75758, 0.76768, 0.77778,
  0.78788, 0.79798, 0.80808, 0.81818, 0.82828, 0.83839, 0.84849,
  0.85859, 0.86869, 0.87879, 0.88889, 0.89899, 0.90909, 0.91919,
  0.92929, 0.93939, 0.94949, 0.95959, 0.96969, 0.97979, 0.98989, 1
};

const double HttpFileSizeRandomVariableImpl::REQUEST_INCDF_NOT_MODIFIED_Y[] = {
  50.9777, 93.8217, 123.301, 146.594, 168.813, 184.263, 193.95,
  200.037, 204.696, 210.069, 217.623, 226.668, 236.226, 245.269,
  253.057, 259.624, 265.16, 269.811, 273.754, 277.165, 280.193,
  283.062, 285.893, 288.79, 291.744, 294.738, 297.754, 300.776,
  303.784, 306.763, 309.698, 312.564, 315.349, 318.042, 320.624,
  323.107, 325.49, 327.782, 329.987, 332.11, 334.159, 336.135, 338.047,
  339.898, 341.695, 343.443, 345.145, 346.808, 348.439, 350.04,
  351.619, 353.177, 354.716, 356.239, 357.744, 359.233, 360.709,
  362.17, 363.619, 365.056, 366.483, 367.9, 369.31, 370.711, 372.106,
  373.496, 374.881, 376.263, 377.643, 379.021, 380.399, 381.778,
  383.159, 384.542, 385.929, 387.321, 388.719, 390.121, 391.528,
  392.939, 394.355, 395.775, 397.199, 398.626, 400.057, 401.491,
  402.928, 404.368, 405.81, 407.255, 408.702, 410.151, 411.602,
  413.054, 414.508, 415.963, 417.418, 418.874, 420.331, 421.788
};

const double HttpFileSizeRandomVariableImpl::REQUEST_INCDF_CONTENT_X[] = {
  -17.7791, -17.5996, -17.4201, -17.2406, -17.0611, -16.8816, -16.7021,
  -16.5226, -16.3431, -16.1635, -15.984, -15.8045, -15.625, -15.4455,
  -15.266, -15.0865, -14.907, -14.7275, -14.548, -14.3685, -14.189,
  -14.0095, -13.83, -13.6505, -13.471, -13.2915, -13.112, -12.9325,
  -12.753, -12.5735, -12.394, -12.2145, -12.035, -11.8555, -11.676,
  -11.4965, -11.317, -11.1375, -10.958, -10.7785, -10.599, -10.4194,
  -10.24, -10.0604, -9.88094, -9.70144, -9.52193, -9.34243, -9.16293,
  -8.98342, -8.80392, -8.62442, -8.44491, -8.26541, -8.08591, -7.9064,
  -7.7269, -7.5474, -7.3679, -7.18839, -7.00889, -6.82939, -6.64988,
  -6.47038, -6.29088, -6.11137, -5.93187, -5.75237, -5.57286, -5.39336,
  -5.21386, -5.03435, -4.85485, -4.67535, -4.49584, -4.31634, -4.13684,
  -3.95733, -3.77783, -3.59833, -3.41883, -3.23932, -3.05982, -2.88032,
  -2.70081, -2.52131, -2.34181, -2.1623, -1.9828, -1.8033, -1.62379,
  -1.44429, -1.26479, -1.08528, -0.90578, -0.72628, -0.54678, -0.36727,
  -0.18777, -0.00827
};

const double HttpFileSizeRandomVariableImpl::REQUEST_INCDF_CONTENT_Y[] = {
  19.7578, 19.6369, 19.5159, 19.3949, 19.274, 19.153, 19.032, 18.9111,
  18.7901, 18.6691, 18.5482, 18.4272, 18.3063, 18.1853, 18.0643,
  17.9434, 17.8224, 17.7014, 17.5805, 17.4595, 17.3386, 17.2176,
  17.0966, 16.9757, 16.8547, 16.7337, 16.6128, 16.4918, 16.3708,
  16.2499, 16.1289, 16.0079, 15.887, 15.766, 15.6451, 15.5241, 15.4031,
  15.2822, 15.1612, 15.0402, 14.9193, 14.7983, 14.6774, 14.5564,
  14.4354, 14.3145, 14.1935, 14.0725, 13.9516, 13.8306, 13.7096,
  13.5887, 13.4677, 13.3468, 13.2258, 13.1048, 12.9839, 12.8629,
  12.7419, 12.621, 12.5, 12.3791, 12.2581, 12.1371, 12.0162, 11.8952,
  11.7742, 11.6533, 11.5323, 11.4113, 11.2904, 11.1694, 11.0485,
  10.9275, 10.8065, 10.6856, 10.5646, 10.4436, 10.3239, 10.2092,
  10.1034, 10.0105, 9.93459, 9.87896, 9.8393, 9.80562, 9.76884,
  9.71919, 9.64712, 9.54737, 9.42776, 9.29852, 9.16992, 9.05219,
  8.95525, 8.88155, 8.82669, 8.78593, 8.75459, 8.72795
};

const double HttpFileSizeRandomVariableImpl::REQUEST_PROB_A = 0.5;
const double HttpFileSizeRandomVariableImpl::REQUEST_D = 0.31;
const double HttpFileSizeRandomVariableImpl::REQUEST_VARRATIO_INTERCEPT = 0.123;
const double HttpFileSizeRandomVariableImpl::REQUEST_VARRATIO_SLOPE = 0.494;

//const double HttpFileSizeRandomVariableImpl::WEIBULLSCALECACHERUN = 1.1341;
//
//const double HttpFileSizeRandomVariableImpl::WEIBULLSHAPECACHERUN_ASYMPTOE = 0.82;
//const double HttpFileSizeRandomVariableImpl::WEIBULLSHAPECACHERUN_PARA1 = 0.6496;
//const double HttpFileSizeRandomVariableImpl::WEIBULLSHAPECACHERUN_PARA2 = 0.0150;
//const double HttpFileSizeRandomVariableImpl::WEIBULLSHAPECACHERUN_PARA3 = 1.5837;
//
//const double HttpFileSizeRandomVariableImpl::WEIBULLSCALEDOWNLOADRUN = 3.0059;
//const double HttpFileSizeRandomVariableImpl::WEIBULLSHAPEDOWNLOADRUN = 0.82;
//
//const double HttpFileSizeRandomVariableImpl::RESPONSE_PARA[] = {
//  WEIBULLSHAPECACHERUN_ASYMPTOE,
//  WEIBULLSHAPECACHERUN_PARA1,
//  WEIBULLSHAPECACHERUN_PARA2,
//  WEIBULLSHAPECACHERUN_PARA3
//};
//
//const double* HttpFileSizeRandomVariableImpl::P = RESPONSE_PARA;
//const uint32_t HttpFileSizeRandomVariableImpl::RESPONSE_CACHE_CUTOFF = 275;
//const uint32_t HttpFileSizeRandomVariableImpl::RESPONSE_STRETCH_THRES = 400;
//const double HttpFileSizeRandomVariableImpl::M_RESPONSE_NOTCACHE = 10.50;
//const double HttpFileSizeRandomVariableImpl::V_RESPONSE_NOTCACHE = 3.23;
//const double HttpFileSizeRandomVariableImpl::M_LOC = 10.62;
//const double HttpFileSizeRandomVariableImpl::V_LOC = 0.94;
//const double HttpFileSizeRandomVariableImpl::SHAPE_SCALE2 = 3.22;
//const double HttpFileSizeRandomVariableImpl::RATE_SCALE2 = 3.22;
//const double HttpFileSizeRandomVariableImpl::V_ERROR = 2.43;

struct ArimaParams HttpFileSizeRandomVariableImpl::fileSizeArimaParams  = {
  REQUEST_D,        // d
  5000,             // number of finite AR rep of farima
  0,                // varRatioParam0
  0,                // varRatioParam1
  0, 1              // pAR=0, qMA
};

Slope HttpFileSizeRandomVariableImpl::fsize_invcdf[2][2] = {
  { Slope (RESPONSE_INCDF_NOT_MODIFIED_X, RESPONSE_INCDF_NOT_MODIFIED_Y,
           sizeof(RESPONSE_INCDF_NOT_MODIFIED_X) / sizeof(double)),
    Slope (RESPONSE_INCDF_CONTENT_X, RESPONSE_INCDF_CONTENT_Y,
           sizeof(RESPONSE_INCDF_CONTENT_X) / sizeof(double))},
  { Slope (REQUEST_INCDF_NOT_MODIFIED_X, REQUEST_INCDF_NOT_MODIFIED_Y,
           sizeof(REQUEST_INCDF_NOT_MODIFIED_X) / sizeof(double)),
    Slope (REQUEST_INCDF_CONTENT_X, REQUEST_INCDF_CONTENT_Y,
           sizeof(REQUEST_INCDF_CONTENT_X) / sizeof(double))}
};

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//HttpNumPagesRandomVariable

HttpNumPagesRandomVariableImpl::HttpNumPagesRandomVariableImpl (double prob, double shape, double scale)
  : m_probability (prob),
    m_shape (shape),
    m_scale (scale)
{
}

HttpNumPagesRandomVariableImpl::~HttpNumPagesRandomVariableImpl ()
{
}

double HttpNumPagesRandomVariableImpl::GetValue (void)
{
  WeibullVariable weibull (1.0, m_shape, m_scale);
  double pages = 1;
  if (rBernoulli (m_probability) == 0)
    {
      // multiple pages in this connection
      pages = weibull.GetValue () + 1;
      if (pages == 1)
        {
          // should be at least 2 pages at this point
          pages++;
        }
    }
  return (pages);
}

double HttpNumPagesRandomVariableImpl::Average ()
{
  return 0;
}

const double HttpNumPagesRandomVariableImpl::P_1PAGE = 0.82;
const double HttpNumPagesRandomVariableImpl::SHAPE_NPAGE = 1;
const double HttpNumPagesRandomVariableImpl::SCALE_NPAGE = 0.417;

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//HttpSingleObjRandomVariableImpl

HttpSingleObjRandomVariableImpl::HttpSingleObjRandomVariableImpl ()
  : m_probability (P_1TRANSFER)
{
}

HttpSingleObjRandomVariableImpl::~HttpSingleObjRandomVariableImpl ()
{
}

HttpSingleObjRandomVariableImpl::HttpSingleObjRandomVariableImpl (double prob)
  : m_probability (prob)
{
}

double HttpSingleObjRandomVariableImpl::GetValue (void)
{
  return (rBernoulli (m_probability));
}

double HttpSingleObjRandomVariableImpl::Average ()
{
  return 0;
}

const double HttpSingleObjRandomVariableImpl::P_1TRANSFER = 0.69;

//-------------------------------------------------------------
//-------------------------------------------------------------
//Http ObjsPerPage RanVar
HttpObjsPerPageRandomVariableImpl::HttpObjsPerPageRandomVariableImpl ()
  : m_shape (SHAPE_NTRANSFER),
    m_scale (SCALE_NTRANSFER)
{
}

HttpObjsPerPageRandomVariableImpl::HttpObjsPerPageRandomVariableImpl (double shape,
                                                                      double scale)
  : m_shape (shape),
    m_scale (scale)
{
}

HttpObjsPerPageRandomVariableImpl::~HttpObjsPerPageRandomVariableImpl ()
{
}

double HttpObjsPerPageRandomVariableImpl::GetValue (void)
{
  WeibullVariable weibull (1.0, m_shape, m_scale);
  return (weibull.GetValue () + 1);
}

double HttpObjsPerPageRandomVariableImpl::Average ()
{
  return 0;
}

const double HttpObjsPerPageRandomVariableImpl::SHAPE_NTRANSFER = 1;
const double HttpObjsPerPageRandomVariableImpl::SCALE_NTRANSFER = 1.578;

//-------------------------------------------------------------
//-------------------------------------------------------------
//Http TimeBtwnPages Random Variable
HttpTimeBtwnPagesRandomVariableImpl::HttpTimeBtwnPagesRandomVariableImpl ()
{
  m_loc_b = m_normal.GetValue () * sqrt (V_LOC_B) + M_LOC_B;
  m_scale2_b = m_gamma.GetValue (SHAPE_SCALE2_B, 1 / RATE_SCALE2_B);
}

HttpTimeBtwnPagesRandomVariableImpl::~HttpTimeBtwnPagesRandomVariableImpl ()
{
}

double HttpTimeBtwnPagesRandomVariableImpl::GetValue (void)
{
  return (pow (2.0, m_loc_b + sqrt (m_scale2_b) * m_normal.GetValue () * sqrt (V_ERROR_B)));
}

double HttpTimeBtwnPagesRandomVariableImpl::Average ()
{
  return 0;
}

const double HttpTimeBtwnPagesRandomVariableImpl::M_LOC_B = 3.22;
const double HttpTimeBtwnPagesRandomVariableImpl::V_LOC_B = 0.73;
const double HttpTimeBtwnPagesRandomVariableImpl::SHAPE_SCALE2_B = 1.85;
const double HttpTimeBtwnPagesRandomVariableImpl::RATE_SCALE2_B = 1.85;
const double HttpTimeBtwnPagesRandomVariableImpl::V_ERROR_B = 1.21;

//-------------------------------------------------------------
//-------------------------------------------------------------
//Http TimeBtwnObjs Random Variable
HttpTimeBtwnObjsRandomVariableImpl::HttpTimeBtwnObjsRandomVariableImpl ()
{
  m_loc_w = m_normal.GetValue () * sqrt (V_LOC_W) + M_LOC_W;
  m_scale2_w = m_gamma.GetValue (SHAPE_SCALE2_W, 1 / RATE_SCALE2_W);
}

HttpTimeBtwnObjsRandomVariableImpl::~HttpTimeBtwnObjsRandomVariableImpl ()
{
}

double HttpTimeBtwnObjsRandomVariableImpl::GetValue (void)
{
  return (pow (2.0, m_loc_w + sqrt (m_scale2_w) * m_normal.GetValue () * sqrt (V_ERROR_W)));
}

double HttpTimeBtwnObjsRandomVariableImpl::Average ()
{
  return 0;
}

const double HttpTimeBtwnObjsRandomVariableImpl::M_LOC_W = -4.15;
const double HttpTimeBtwnObjsRandomVariableImpl::V_LOC_W = 3.12;
const double HttpTimeBtwnObjsRandomVariableImpl::SHAPE_SCALE2_W = 2.35;
const double HttpTimeBtwnObjsRandomVariableImpl::RATE_SCALE2_W = 2.35;
const double HttpTimeBtwnObjsRandomVariableImpl::V_ERROR_W = 1.57;

//-------------------------------------------------------------
//-------------------------------------------------------------
//Http ServerDelay Random Variable
HttpServerDelayRandomVariableImpl::HttpServerDelayRandomVariableImpl (double shape,
                                                                      double scale)
  : m_shape (shape),
    m_scale (scale),
    m_const (1),
    m_mean (0)
{
}

HttpServerDelayRandomVariableImpl::~HttpServerDelayRandomVariableImpl ()
{
}

double HttpServerDelayRandomVariableImpl::GetValue (void)
{
  /*
   *  Sample from Weibull distribution.
   *  delay is 1/sample, with a max delay of 0.1 sec
   */
  WeibullVariable weibull (1.0, m_shape, m_scale);
  double val = weibull.GetValue ();

  /*
   * To avoid generating very large delays, we limit the maximum delay to 10 secs
   */
  if (val < 10)
    {
      val = 10;
    }

  // adjust by m_const and m_mean (by default, this does nothing)
  if (m_const == 0)
    {
      m_const = m_mean / SERVER_DELAY_DIV;
    }
  val = val / m_const;

  return (1 / val);        // delay in seconds
}

double HttpServerDelayRandomVariableImpl::Average ()
{
  return 0;
}

const double HttpServerDelayRandomVariableImpl::SERVER_DELAY_SHAPE = 0.63;
const double HttpServerDelayRandomVariableImpl::SERVER_DELAY_SCALE = 305;
const double HttpServerDelayRandomVariableImpl::SERVER_DELAY_DIV = 0.0059;

} // namespace http
} // namespace ns3
