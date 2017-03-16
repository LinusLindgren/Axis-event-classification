% x = kf1d(y, mx, cfg) implements a simple 1d Kalman filter.
% x <- filtered output
% y <- measured data
% mx <- expected value of the input process
% cfg is a structure contiaining:
%    .M <- scaling factor (usually 1)
%    .sigma_x2 <- variance of the process noise (> 0)
%    .sigma_e2 <- variance of the measurement noise (> 0)

function x = kf1d(y, mx, cfg)

  if (isvector(y) == 0)
    printf("Error: input has to be a vector!");
    return;
  end
    
  sx2 = cfg.sigma_x2;
  se2 = cfg.sigma_e2;
  M = cfg.M;
  x = zeros(size(y));
  % Kalman Filter Loop
  for ii = 1:length(y),
    % calculate the Kalman gain
    K = M*sx2 / (M^2*sx2 + se2);
    % compute the new estimate
    x(ii) = mx + K*(y(ii)-M*mx);
    % update the variance
    sx2 = (1 - K*M)*sx2;
    mx = x(ii);
  end
