% x = read_tilt_data_from_file(filename)
% reads tilt values from a file.
% the entries of the file are assumed to be filled
% by rows.
function x = read_tilt_data_from_file(filename)
  %x = load(filename);
  % we assume that tilt values are filled one row at a time.
  [start_X, start_y,start_z] = textread (filename, "startup(x,y,z):(%d,%d,%d)", 1);
  [x_values, y_values,z_values] = textread (filename, "%d %d %d","headerlines",1 );
  
  x = zeros(256,1);
  index = 1;
  step_size = 2; % to convert to 200 HZ from 400 HZ
  start_tilt = atan2(start_y, start_z) * 180 / pi;
  for i = 1: 256
    x(i) = atan2(y_values(index), z_values(index)) * 180 / pi - start_tilt;
    index = index+step_size;
  end
  
  %x = x'; x = x(:);
  return;

