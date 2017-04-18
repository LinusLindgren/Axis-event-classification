% 
% Tampering Detection Algorithm
% This script can be used to train a model or classify events
% Output of training: param.dat
% Output of classification: classified.ssv
%
clear all; close all; clc

% Parameters.

action = "train";                       % "train" OR  "classify"
L = 10;                                    % moving average filter length
T = 128;                                   % Length of the data sequence
NFFT = 128;                               % number of FFT points (should be a power of 2)
NPEAKSMAX = 4;                             % max. number of peaks to consider
PTHRESH = 3;                               % Tp, if power < Tp => Noise. 
fs = 200;                                  % Hz, sampling frequency
%dirname = 't3/';
dirname = 'samples/';                          % Folder name for the train/classify data set.
training_set = 'train/acc*';               % training set
test_set = 'test/acc*';                   % test (classification) set.

if (strcmpi(action, "train") == 1)
  %
  % Training.
  %
  filenames = dir([dirname,training_set]);
  N = length(filenames);
  printf("\nNumber of training data sets: %d\n", N);
  idx=0;
  for ii = 1:N
    filename = [dirname, 'train/', filenames(ii).name];
    % read the raw tilt values
    raw_tilt = read_tilt_data_from_file(filename);

    % filter using a MAF of filter length L
    filt_tilt = maf(raw_tilt, L);

    % consider only 128 values from the filter output
    filt_tilt = filt_tilt(1:T);

    % check if power > threshold. if not, continue to next.
    pwr = sqrt(1/T * sum(filt_tilt.^2));
    if (pwr < PTHRESH)
      continue;
    end
    idx=idx+1;

    % calculate the spectrum (periodogram)
    [fft_out, spectrum] = periodogram(filt_tilt, NFFT, 0);

    % find peak locations and peak values (ignore DC)
    [plocs, pvals] = findpeaks(spectrum(1:NFFT/2), NPEAKSMAX);
    % limit to 6dB range.
    pidx = find(pvals < 0.25*pvals(1),1);
    if not(isempty(pidx))
      plocs = plocs(1:pidx); pvals = pvals(1:pidx);
    end

    % store the 1st and 2nd strongest frequencies.
    f1(idx) = findf(plocs,NFFT,fs,1);
    f2 = findf(plocs,NFFT,fs,2);
    if (not(isempty(f2)))
      fdiff(ii) = abs(f2-f1(idx));
    end

    % calculate peak ratio metric (PR)
    pratio(idx) = calc_peak_ratio(spectrum(1:NFFT/2), plocs);

    % calculate peak width metric (PW)
    pwidths(idx) = mean(calc_peak_width(spectrum, plocs));
  end

  % compute histograms and store.
  % for frequencies f1 and f2
  param.fbins = linspace(1.56,20,15);
  [nf1,xf1] = hist(f1,param.fbins);
  [nf2,xf2] = hist(fdiff);
  param.fdiffbins = xf2;
  param.f1p = nf1./max(nf1);
  param.f2p = nf2./max(nf2);

  % for peak widths
  param.pwbins = 10:5:150;
  [npw,xpw] = hist(pwidths, param.pwbins);
  param.pwp = npw./max(npw);

  % for peak ratios
  [npr, xpr] = hist(pratio, 10);
  param.prbins = xpr;
  param.prp = npr./max(npr);

  save param.dat param;

  print_param(param);
  
elseif (strcmpi(action, "classify") == 1)
  %
  % Classification.
  %
  if (not(exist('param.dat')))
    printf("\nERROR: No model exists (param.dat)\n");
    return;
  end
  param = load('param.dat');
  param = param.param;
  printf("\nUsing Parameters:");
  print_param(param);
  filenames = dir([dirname,test_set]);
  N = length(filenames);
  ofilename = [dirname,"classified.ssv"];
  fid = fopen(ofilename, 'w');
  for ii = 1:N
    filename = [dirname,filenames(ii).name];
    printf("\nFile: %s ", filename);
    % read the raw tilt values
    raw_tilt = read_tilt_data_from_file(filename);

    % filter using a MAF of filter length L
    filt_tilt = maf(raw_tilt, L);

    % consider only 128 values from the filter output
    filt_tilt = filt_tilt(1:T);

    % check if power > threshold. if not, continue to next.
    pwr = sqrt(1/T * sum(filt_tilt.^2));
    if (pwr < PTHRESH)
      printf("Power: %2.2f < Threshold. Noise.\n", pwr);
      continue;
    end

    % calculate the spectrum (periodogram)
    [fft_out, spectrum] = periodogram(filt_tilt, NFFT, 0);

    % find peak locations and peak values (ignore DC)
    [plocs, pvals] = findpeaks(spectrum(1:NFFT/2), 2);
    % limit to 6dB range.
    pidx = find(pvals < 0.25*pvals(1),1);
    if not(isempty(pidx))
      plocs = plocs(1:pidx); pvals = pvals(1:pidx);
    end

    % find the score for f1 and f2
    f1 = findf(plocs,NFFT,fs,1);
    F1SCORE = compute_score(param.fbins, param.f1p, f1);

    f2 = findf(plocs,NFFT,fs,2);
    if (not(isempty(f2)))
      fdiff = abs(f2-f1);
      FDIFFSCORE = compute_score(param.fdiffbins, param.f2p, fdiff);
    else
      f2 = -1; fdiff = -1;
      FDIFFSCORE = -1.0;
    end

    % calculate peak ratio metric (PR)
    pratio = calc_peak_ratio(spectrum(1:NFFT/2), plocs);
    PRSCORE = compute_score(param.prbins, param.prp, pratio);

    % calculate peak width metric (PW)
    pwidth = mean(calc_peak_width(spectrum, plocs));
    PWSCORE = compute_score(param.pwbins, param.pwp, pwidth);

    fprintf(fid, "%s %2.2f %2.2f %2.2f %2.2f %2.2f %2.2f %2.2f %2.2f %2.2f\n", filename, f1, f2, fdiff, pratio, pwidth, F1SCORE, FDIFFSCORE, PRSCORE, PWSCORE);

  end
  
  fclose(fid);
  
else
  printf("ERROR: Unknown mode");
end
