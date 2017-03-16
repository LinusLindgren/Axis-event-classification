% x = read_tilt_data_from_file(filename)
% reads tilt values from a file.
% the entries of the file are assumed to be filled
% by rows.
function x = read_tilt_data_from_file(filename)
  x = load(filename);
  % we assume that tilt values are filled one row at a time.
  x = x'; x = x(:);
  return;

