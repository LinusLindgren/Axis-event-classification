% y = maf(x,L)
% implements a moving average filter of length L
% x <- input data
% L <- Length of the moving average filter (>0)
% y <- output
function y = maf(x, L)
  if (L < 1)
    y = [];
    disp('Error: invalid length!');
    return;
  end
  b = ones(1,L)/L;
  y = filter(b,1,x);
