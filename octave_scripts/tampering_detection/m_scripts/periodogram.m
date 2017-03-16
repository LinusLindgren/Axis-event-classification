% [xf, x_per] = periodogram(x, remove_mean)
% computes the periodogram of the data x
% remove_mean: 1 = remove mean(x) from x before calculating periodogram.
% xf : Raw FFT (complex valued)
% x_per : Periodogram = 1/N |X(f)|^2
function [xf, x_per] = periodogram(x, remove_mean)
  if (remove_mean)
    x = x - mean(x);
  end
  xf = fft(x);
  x_per = 1/length(x) * abs(xf).^2;
