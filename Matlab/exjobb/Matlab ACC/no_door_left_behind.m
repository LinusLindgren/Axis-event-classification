function [ max_neg, new_ratio ] = no_door_left_behind(scores_pos_test ,scores_neg_test  )

max_neg = max(scores_neg_test);
i = 0;
new_ratio = [];
while i < max_neg
    pos_ratio = size(scores_pos_test(scores_pos_test >= i),1) / size(scores_pos_test,1);
    neg_ratio = size(scores_neg_test(scores_neg_test < i),1) / size(scores_neg_test,1);
    new_ratio = [new_ratio ; [i pos_ratio neg_ratio]];
    i= i+0.1;
end
end

